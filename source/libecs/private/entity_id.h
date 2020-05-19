// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/ecs/common.h"
#include <potato/spud/int_types.h>

namespace up {
    static constexpr auto makeEntityId(uint32 mappingIndex, uint32 generation) noexcept -> EntityId {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 32) | mappingIndex);
    }

    static constexpr auto getEntityMappingIndex(EntityId entity) noexcept -> uint32 {
        return static_cast<uint64>(entity) & 0xFFFFFFFF;
    }

    static constexpr auto getEntityGeneration(EntityId entity) noexcept -> uint32 {
        return static_cast<uint64>(entity) >> 32;
    }

    static constexpr auto makeMapped(uint16 generation, uint16 archetypeIndex, uint16 chunkIndex, uint16 entityIndex) -> uint64 {
        return (static_cast<uint64>(generation) << 48) | (static_cast<uint64>(archetypeIndex) << 32) | (static_cast<uint64>(chunkIndex) << 16) | static_cast<uint64>(entityIndex);
    }

    static constexpr auto getMappedChunk(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 16);
    }

    static constexpr auto getMappedIndex(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping);
    }

    static constexpr auto getMappedGeneration(uint64 mapping) noexcept -> uint32 {
        return static_cast<uint32>(mapping >> 48);
    }

    static constexpr auto getMappedArchetype(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 32);
    }

    static constexpr auto makeFreeEntry(uint16 generation, uint32 index) noexcept -> uint64 {
        return (static_cast<uint64>(generation) << 48) | static_cast<uint64>(index);
    }
} // namespace up
