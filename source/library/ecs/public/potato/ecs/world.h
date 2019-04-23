// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"

namespace up {
    using RawSelectSignature = void(size_t count, EntityId const* entities, view<void*> componentArrays);
    template <typename... Components>
    using TypedSelectSignature = void(size_t count, EntityId const* entities, Components*... components);

    namespace _detail {
        template <size_t... Indices, typename... Components>
        void innerSelectHelper(std::index_sequence<Indices...>, size_t count, EntityId const* entities, view<void*> arrays, delegate_ref<TypedSelectSignature<Components...>> callback) {
            callback(count, entities, static_cast<Components*>(arrays[Indices])...);
        }

        template <typename... Components>
        void selectHelper(size_t count, EntityId const* entities, view<void*> arrays, delegate_ref<TypedSelectSignature<Components...>> callback) {
            innerSelectHelper(std::make_index_sequence<sizeof...(Components)>(), count, entities, arrays, callback);
        }

    } // namespace _detail

    /// A world contains a collection of entities, archetypes, and their associated components.
    ///
    /// Entities from different Worlds cannot interact.
    class World {
    public:
        struct Archetype;
        struct Chunk;
        struct Entity;

        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        template <typename... Components, typename Callable>
        void select(Callable&& callback) const;

        UP_ECS_API view<box<Archetype>> archetypes() const noexcept;

        template <typename... Components>
        EntityId createEntity(Components const&... components) noexcept;
        UP_ECS_API void deleteEntity(EntityId entity) noexcept;

        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept;
        UP_ECS_API void* getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept;

    private:
        
        UP_ECS_API void _selectRaw(view<ComponentId> components, delegate_ref<RawSelectSignature> callback) const;
        UP_ECS_API EntityId _createEntityRaw(view<ComponentMeta const*> components, view<void const*> data);
        UP_ECS_API EntityId _allocateEntityId(uint32 archetypeIndex, uint32 entityIndex) noexcept;

        void _calculateLayout(uint32 archetypeIndex, view<ComponentMeta const*> components);
        void* _getComponentPointer(uint32 archetypeIndex, uint32 entityIndex, ComponentId component) const noexcept;
        bool _matchArchetype(uint32 archetypeIndex, view<ComponentId> components) const noexcept;
        bool _matchArchetypeExact(uint32 archetypeIndex, view<ComponentMeta const*> components) const noexcept;
        void _selectChunksRaw(uint32 archetypeIndex, view<ComponentId> components, delegate_ref<RawSelectSignature> callback) const;
        void _recycleEntityId(EntityId entity) noexcept;
        uint32 _findArchetypeIndex(view<ComponentMeta const*> components) noexcept;
        box<Chunk> _allocateChunk();
        void _recycleChunk(box<Chunk>);

        vector<Entity> _entityMapping;
        vector<box<Archetype>> _archetypes;
        Chunk* _freeChunkHead = nullptr;
        uint32 _freeEntityHead = static_cast<uint32>(-1);
    };

    template <typename... Components, typename Callable>
    void World::select(Callable&& callback) const {
        _selectRaw(view<ComponentId>({getComponentId<Components>()...}), [&callback](size_t count, EntityId const* entities, view<void*> arrays) {
            _detail::selectHelper<Components...>(count, entities, arrays, delegate_ref<void(size_t, EntityId const*, Components*...)>(std::forward<Callable>(callback)));
        });
    }

    template <typename... Components>
    EntityId World::createEntity(Components const&... components) noexcept {
        ComponentMeta const* componentMetas[] = {ComponentMeta::get<Components>()...};
        void const* componentData[] = {&components...};

        return _createEntityRaw(componentMetas, componentData);
    }

    template <typename Component>
    Component* World::getComponentSlow(EntityId entity) noexcept {
        return static_cast<Component*>(getComponentSlowUnsafe(entity, getComponentId<Component>()));
    }
} // namespace up
