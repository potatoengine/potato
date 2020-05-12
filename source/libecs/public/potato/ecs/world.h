// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_detail/ecs_context.h"
#include "_export.h"
#include "chunk.h"
#include "potato/ecs/component.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/rc.h"
#include "potato/spud/box.h"

namespace up {
    template <int Count>
    struct QueryMatch {
        ArchetypeId archetype;
        int offsets[Count];
    };

    /// A world contains a collection of Entities, Archetypes, and their associated Components.
    ///
    /// Entities from different Worlds cannot interact.
    ///
    class World {
    public:
        UP_ECS_API explicit World(_detail::EcsContext& context);
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        /// Retrieve the chunks belonging to a specific archetype.
        ///
        /// @returns nullptr if the ArchetypeId is invalid
        ///
        UP_ECS_API auto chunksOf(ArchetypeId archetype) const noexcept -> view<Chunk*>;

        /// @brief View of all chunks allocated in the world.
        /// @return all chunks in the world.
        auto chunks() const noexcept -> view<Chunk*> { return _chunks; }

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

        /// @brief Add a default-constructed component to an existing entity.
        /// @param entity The entity to add the componet to.
        /// @param meta Metadata for the to-be-added component.
        UP_ECS_API void addComponentDefault(EntityId entity, ComponentMeta const& meta);

        /// Removes a Component from an existing Entity.
        ///
        /// Changes the Entity's Archetype and home Chunk
        ///
        UP_ECS_API void removeComponent(EntityId entityId, ComponentId componentId) noexcept;

        /// @brief Removes a component from an entity.
        /// @tparam Component Component type to remove.
        /// @param entityId Entity to modify.
        template <typename Component>
        void removeComponent(EntityId entityId) noexcept {
            ComponentMeta const* const meta = _context.findComponentByType<Component>();
            return removeComponent(entityId, meta->id);
        }

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

        /// Interrogate an entity and enumerate all of its components.
        ///
        template <typename Callback, typename Void = enable_if_t<is_invocable_v<Callback, EntityId, ArchetypeId, ComponentMeta const*, void*>>>
        auto interrogateEntityUnsafe(EntityId entity, Callback&& callback) const {
            if (auto [success, archetype, chunkIndex, index] = _parseEntityId(entity); success) {
                auto const layout = _context.layoutOf(archetype);
                Chunk* const chunk = _getChunk(archetype, chunkIndex);
                for (ChunkRowDesc const& row : layout) {
                    callback(entity, archetype, row.meta, static_cast<void*>(chunk->payload + row.offset + row.width * index));
                }
                return true;
            }
            return false;
        }

        template <typename... Components>
        auto matchArchetypesInto(size_t firstIndex, bit_set const& mask, vector<QueryMatch<sizeof...(Components)>>& matches) const noexcept -> size_t {
            static ComponentId const components[sizeof...(Components)] = {_context.findComponentByType<Components>()->id...};
            return _context.selectArchetypes(firstIndex, mask, components, [&matches](ArchetypeId arch, view<int> offsets) {
                auto& match = matches.emplace_back();
                match.archetype = arch;
                std::memcpy(&match.offsets, offsets.data(), sizeof(QueryMatch<sizeof...(Components)>::offsets));
            });
        }

    private:
        struct AllocatedLocation {
            Chunk& chunk;
            uint16 chunkIndex;
            uint16 index;
        };

        struct ArchetypeChunkRange {
            uint32 offset = 0;
            uint32 length = 0;
        };

        struct EntityLocation {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
            uint16 chunk = 0;
            uint16 index = 0;
        };

        static constexpr uint32 freeEntityIndex = ~0U;

        UP_ECS_API EntityId _createEntityRaw(view<ComponentMeta const*> components, view<void const*> data);
        UP_ECS_API void _addComponentRaw(EntityId entityId, ComponentMeta const& componentMeta, void const* componentData) noexcept;

        auto _allocateEntitySpace(ArchetypeId archetype) -> AllocatedLocation;
        auto _allocateEntityId(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId;
        void _recycleEntityId(EntityId entity) noexcept;

        UP_ECS_API auto _parseEntityId(EntityId entity) const noexcept -> EntityLocation;

        void _remapEntityId(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept;

        void _moveTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ArchetypeId srcArch, Chunk& srcChunk, int srcIndex);
        void _moveTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex);
        void _copyTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ComponentId srcComponent, void const* srcData);
        void _constructAt(ArchetypeId arch, Chunk& chunk, int index, ComponentId component);
        void _destroyAt(ArchetypeId arch, Chunk& chunk, int index);

        void _addChunk(ArchetypeId archetype, Chunk* chunk);
        void _removeChunk(ArchetypeId archetype, int chunkIndex) noexcept;
        UP_ECS_API auto _getChunk(ArchetypeId archetype, int chunkIndex) const noexcept -> Chunk*;

        vector<ArchetypeChunkRange> _archetypeChunkRanges;
        vector<Chunk*> _chunks;
        vector<uint64> _entityMapping;
        uint32 _freeEntityHead = freeEntityIndex;
        _detail::EcsContext& _context;
    };

    template <typename... Components>
    EntityId World::createEntity(Components const&... components) noexcept {
        ComponentMeta const* const componentMetas[] = {_context.findComponentByType<Components>()...};
        void const* const componentData[] = {&components...};

        return _createEntityRaw(componentMetas, componentData);
    }

    template <typename Component>
    Component* World::getComponentSlow(EntityId entity) noexcept {
        auto meta = _context.findComponentByType<Component>();
        if (meta == nullptr) {
            return nullptr;
        }
        return static_cast<Component*>(getComponentSlowUnsafe(entity, meta->id));
    }

    template <typename Component>
    void World::addComponent(EntityId entityId, Component const& component) noexcept {
        ComponentMeta const* const meta = _context.findComponentByType<Component>();
        _addComponentRaw(entityId, *meta, &component);
    }
} // namespace up
