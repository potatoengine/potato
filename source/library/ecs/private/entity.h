// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Entity {
    uint32 generation = 0;
    uint32 archetype = 0;
    uint32 index = 0;
};

namespace up {
    constexpr EntityId makeEntityId(uint32 mappingIndex, uint32 generation) noexcept {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 32) | mappingIndex);
    }

    constexpr uint32 getEntityMappingIndex(EntityId entity) noexcept {
        return static_cast<uint64>(entity) & 0xFFFFFFFF;
    }

    constexpr uint32 getEntityGeneration(EntityId entity) noexcept {
        return static_cast<uint64>(entity) >> 32;
    }
}
