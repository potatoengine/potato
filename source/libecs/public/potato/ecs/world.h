// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "chunk.h"
#include "potato/ecs/component.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/rc.h"
#include "potato/spud/box.h"

namespace up {
    struct Archetype;

    template <typename... Components>
    class Query;

    /// A world contains a collection of Entities, Archetypes, and their associated Components.
    ///
    /// Entities from different Worlds cannot interact.
    ///
    class World {
    public:
        using ForEachChunkSignature = void(Chunk*);
        using SelectSignature = void(ArchetypeId, view<int>);

        struct Entity;
        struct Layout;
        struct Location;

        /// Maximum number of Components that can be selected (used in a Query)
        ///
        static constexpr uint32 maxSelectComponents = 64;

        /// Maximum number of Components that can exist on an Archetype (and hence any single Entity)
        ///
        static constexpr uint32 maxArchetypeComponents = 256;

        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        uint32 version() const noexcept { return _version; }

        UP_ECS_API view<box<Archetype>> archetypes() const noexcept;

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

        /// Invoke a callback for each chunk in an archetype.
        ///
        UP_ECS_API void forEachChunk(ArchetypeId archetype, delegate_ref<ForEachChunkSignature> callback) const;

        /// Find matching archetypes.
        ///
        /// @return the number of matched archetypes.
        ///
        UP_ECS_API int selectArchetypes(view<ComponentId> components, delegate_ref<SelectSignature> callback) const;

    private:
        UP_ECS_API EntityId _createEntityRaw(view<ComponentMeta const*> components, view<void const*> data);
        UP_ECS_API EntityId _allocateEntityId(uint32 archetypeIndex, uint32 entityIndex) noexcept;
        UP_ECS_API void _addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept;

        void _deleteLocation(Location const& location) noexcept;
        void _populateArchetype(uint32 archetypeIndex, view<ComponentMeta const*> components);
        static void _calculateLayout(Archetype& archetype, size_t size);
        void _selectChunksRaw(uint32 archetypeIndex, uint32 chunkIndex, view<ComponentId> components, size_t& out_count, span<void*> outputPointers) const;
        void _recycleEntityId(EntityId entity) noexcept;
        uint32 _findArchetypeIndex(view<ComponentMeta const*> components) noexcept;

        bool _tryGetLocation(EntityId entityId, Location& out) const noexcept;

        box<Chunk> _allocateChunk();
        void _recycleChunk(box<Chunk>);

        static void* _stream(char* data, uint32 offset, uint32 width, uint32 index) noexcept { return data + offset + width * index; }
        template <typename T> static auto _stream(char* data, uint32 offset, uint32 index) noexcept -> T* { return static_cast<T*>(static_cast<void*>(data + offset + sizeof(T) * index)); }
        static void* _stream(char* data, uint32 offset) noexcept { return data + offset; }

        static constexpr uint32 freeEntityIndex = static_cast<uint32>(-1);

        uint32 _version = 0;
        vector<Entity> _entityMapping;
        vector<box<Archetype>> _archetypes;
        vector<box<Chunk>> _freeChunks;
        uint32 _freeEntityHead = freeEntityIndex;
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
