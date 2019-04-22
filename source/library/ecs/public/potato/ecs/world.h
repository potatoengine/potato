// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
#include "potato/ecs/domain.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"

namespace up {
    struct EntityChunk;
    class Archetype;

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    namespace _detail {
        template <size_t... Indices, typename... Components>
        void innerSelectHelper(std::index_sequence<Indices...>, size_t count, view<void*> arrays, delegate_ref<void(size_t count, Components*... components)> callback) {
            callback(count, static_cast<Components*>(arrays[Indices])...);
        }

        template <typename... Components>
        void selectHelper(size_t count, view<void*> arrays, delegate_ref<void(size_t count, Components*... components)> callback) {
            innerSelectHelper(std::make_index_sequence<sizeof...(Components)>(), count, arrays, callback);
        }

    } // namespace _detail

    /// A world contains a collection of entities, archetypes, and their associated components.
    ///
    /// Entities from different Worlds cannot interact.
    class World {
    public:
        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        template <typename... Components, typename Callable>
        void select(Callable&& callback) const;

        UP_ECS_API Archetype const* acquireArchetype(view<ComponentId> components) noexcept;
        UP_ECS_API view<rc<Archetype>> archetypes() const noexcept;

        template <typename... Components>
        EntityId createEntity(Components const&... components) noexcept;
        UP_ECS_API void deleteEntity(EntityId entity) noexcept;

        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept;

    private:
        UP_ECS_API void* _getComponentPointer(EntityId entity, ComponentId component) noexcept;
        UP_ECS_API void _select(view<ComponentId> components, delegate_ref<SelectSignature> callback) const;
        UP_ECS_API EntityId _createEntity(view<ComponentId> components, view<void const*> data);

        void* _getComponentPointer(uint32 archetypeIndex, uint32 entityIndex, ComponentId component) const noexcept;
        bool _matchArchetype(uint32 archetypeIndex, view<ComponentId> components) const noexcept;
        bool _matchArchetypeExact(uint32 archetypeIndex, view<ComponentId> components) const noexcept;
        void _deleteEntity(uint32 archetypeIndex, uint32 entityIndex) noexcept;
        uint32 _createArchetypeEntity(uint32 archetypeIndex, EntityId entity, view<ComponentId> componentIds, view<void const*> componentData) noexcept;
        void _selectArchetype(uint32 archetypeIndex, view<ComponentId> components, delegate_ref<SelectSignature> callback) const;
        EntityId _allocateEntityId() noexcept;
        void _recycleEntityId(EntityId entity) noexcept;
        uint32 _findArchetypeIndex(view<ComponentId> components) noexcept;
        box<EntityChunk> _allocateChunk();
        void _recycleChunk(box<EntityChunk>);

        vector<EntityMapping> _entityMapping;
        vector<rc<Archetype>> _archetypes;
        vector<box<EntityChunk>> _chunkPool;
        uint32 _freeEntityHead = static_cast<uint32>(-1);
    };

    template <typename... Components, typename Callable>
    void World::select(Callable&& callback) const {
        _select(view<ComponentId>({getComponentId<Components>()...}), [&callback](size_t count, view<void*> arrays) {
            _detail::selectHelper<Components...>(count, arrays, delegate_ref<void(size_t, Components * ...)>(std::forward<Callable>(callback)));
        });
    }

    template <typename... Components>
    EntityId World::createEntity(Components const&... components) noexcept {
        ComponentId const componentIds[] = {getComponentId<Components>()...};
        void const* componentData[] = {&components...};

        return _createEntity(componentIds, componentData);
    }

    template <typename Component>
    Component* World::getComponentSlow(EntityId entity) noexcept {
        return static_cast<Component*>(_getComponentPointer(entity, getComponentId<Component>()));
    }
} // namespace up
