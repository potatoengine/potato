// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/int_types.h"

namespace up {
    enum class EntityId : uint64 { None = 0 };

    constexpr EntityId makeEntityId(uint32 index, uint32 generation) noexcept {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 32) | index);
    }

    constexpr uint32 getEntityIndex(EntityId entity) noexcept {
        return static_cast<uint64>(entity) & 0xFFFFFFFF;
    }

    constexpr uint32 getEntityGeneration(EntityId entity) noexcept {
        return static_cast<uint64>(entity) >> 32;
    }
}
