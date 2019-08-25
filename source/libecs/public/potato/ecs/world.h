// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "chunk.h"
#include "entity_mapper.h"
#include "archetype.h"
#include "potato/ecs/component.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/rc.h"
#include "potato/spud/box.h"

namespace up {
    /// A world contains a collection of Entities, Archetypes, and their associated Components.
    ///
    /// Entities from different Worlds cannot interact.
    ///
    class World {
    public:
        using SelectSignature = void(ArchetypeId, view<int>);

        struct Location;

        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        /// Indicates the structure of the World (archetypes) has changed
        ///
        /// Does *not* indicate anything about the creation or moving of entities
        ///
        UP_ECS_API uint32 version() const noexcept;

        UP_ECS_API view<box<Archetype>> archetypes() const noexcept;

        /// Retrieve a specific Archetype
        ///
        /// @returns nullptr if the ArchetypeId is invalid
        ///
        UP_ECS_API auto getArchetype(ArchetypeId arch) const noexcept -> Archetype const*;

        /// Creates a new Entity with the provided list of Component data
        ///
        template <typename... Components>
        EntityId createEntity(Components const&... components) noexcept;

        /// Deletes an existing Entity
        ///
        UP_ECS_API void deleteEntity(EntityId entity) noexcept;

        /// Adds a new Component to an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        template <typename Component>
        void addComponent(EntityId entityId, Component const& component) noexcept;

        /// Removes a Component from an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        UP_ECS_API void removeComponent(EntityId entityId, ComponentId componentId) noexcept;

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is typically a slow operation. It will incur several table lookups
        /// and searches. This should only be used by tools and debug aids, typically,
        /// and a Query should be used for runtime code.
        ///
        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept;

        /// Retrieves a pointer to a Component on the specified Entity.
        ///
        /// This is a type-unsafe variant of getComponentSlow.
        ///
        UP_ECS_API void* getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept;

        /// Find matching archetypes.
        ///
        /// @return the number of matched archetypes.
        ///
        UP_ECS_API int selectArchetypes(view<ComponentId> components, delegate_ref<SelectSignature> callback) const;

    private:
        UP_ECS_API EntityId _createEntityRaw(view<ComponentMeta const*> components, view<void const*> data);
        UP_ECS_API void _addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept;

        void _deleteLocation(Location const& location) noexcept;
        bool _tryGetLocation(EntityId entityId, Location& out) const noexcept;

        EntityMapper _entities;
        ArchetypeMapper _archetypes;
        ChunkAllocator _chunks;
    };

    template <typename... Components>
    EntityId World::createEntity(Components const&... components) noexcept {
        ComponentMeta const* const componentMetas[] = {ComponentMeta::get<Components>()...};
        void const* const componentData[] = {&components...};

        return _createEntityRaw(componentMetas, componentData);
    }

    template <typename Component>
    Component* World::getComponentSlow(EntityId entity) noexcept {
        return static_cast<Component*>(getComponentSlowUnsafe(entity, getComponentId<Component>()));
    }

    template <typename Component>
    void World::addComponent(EntityId entityId, Component const& component) noexcept {
        _addComponentRaw(entityId, ComponentMeta::get<Component>(), &component);
    }
} // namespace up
