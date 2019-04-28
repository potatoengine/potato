// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"

namespace up {
    template <typename... Components>
    class Query;

    /// A world contains a collection of entities, archetypes, and their associated components.
    ///
    /// Entities from different Worlds cannot interact.
    class World {
    public:
        using RawSelectSignature = void(size_t, EntityId const*, view<void*>);

        struct Archetype;
        struct Chunk;
        struct Entity;
        struct Layout;
        struct Location;

        static constexpr uint32 maxSelectComponents = 64;
        static constexpr uint32 maxArchetypeComponents = 256;

        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        template <typename... Components>
        void select(Query<Components...>& query) const;

        UP_ECS_API view<box<Archetype>> archetypes() const noexcept;

        template <typename... Components>
        EntityId createEntity(Components const&... components) noexcept;
        UP_ECS_API void deleteEntity(EntityId entity) noexcept;

        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept;
        UP_ECS_API void* getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept;

    private:
        UP_ECS_API void _selectRaw(view<ComponentId> sortedComponents, delegate_ref<RawSelectSignature> callback) const;
        UP_ECS_API EntityId _createEntityRaw(view<ComponentMeta const*> components, view<void const*> data);
        UP_ECS_API EntityId _allocateEntityId(uint32 archetypeIndex, uint32 entityIndex) noexcept;

        void _calculateLayout(uint32 archetypeIndex, view<ComponentMeta const*> components);
        bool _matchArchetype(uint32 archetypeIndex, view<ComponentId> sortedComponents) const noexcept;
        void _selectChunksRaw(uint32 archetypeIndex, view<ComponentId> components, delegate_ref<RawSelectSignature> callback) const;
        void _recycleEntityId(EntityId entity) noexcept;
        uint32 _findArchetypeIndex(view<ComponentMeta const*> components) noexcept;
        box<Chunk> _allocateChunk();
        void _recycleChunk(box<Chunk>);

        bool _tryGetLocation(EntityId entityId, Location& out) const noexcept;

        vector<Entity> _entityMapping;
        vector<box<Archetype>> _archetypes;
        Chunk* _freeChunkHead = nullptr;
        uint32 _freeEntityHead = static_cast<uint32>(-1);
    };

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

    template <typename... Components>
    void World::select(Query<Components...>& query) const {
        _selectRaw(query.components(), [&query](size_t count, EntityId const* entities, view<void*> pointers) {
            query.invokeUnsafe(count, entities, pointers);
        });
    }
} // namespace up
