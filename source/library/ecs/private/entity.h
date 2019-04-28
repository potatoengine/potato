// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

/// Mapping information about where a particular Entity lives in the World
struct up::World::Entity {
    uint32 generation = 0;
    uint32 archetype = 0;
    uint32 index = 0;
};

namespace up {
    /// Creates a new EntityId, without yet knowing its Archetype
    constexpr EntityId makeEntityId(uint32 mappingIndex, uint32 generation) noexcept {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 32) | mappingIndex);
    }

    /// Retrieves the mapping index from an EntityId (the index into the _entityMapping table)
    constexpr uint32 getEntityMappingIndex(EntityId entity) noexcept {
        return static_cast<uint64>(entity) & 0xFFFFFFFF;
    }

    /// Retrieves the generation of an EntityId (used to detect stale EntityId handles)
    constexpr uint32 getEntityGeneration(EntityId entity) noexcept {
        return static_cast<uint64>(entity) >> 32;
    }
}
