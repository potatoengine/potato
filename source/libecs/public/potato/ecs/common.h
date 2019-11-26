// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/int_types.h"

namespace up {
    /// Unique identifier for an Archetype
    enum class ArchetypeId : uint32 { Empty = 0 };

    /// Unique identifier for a Component
    enum class ComponentId : uint64 { Unknown = 0 };

    /// Unique identifier for an Entity
    enum class EntityId : uint64 { None = 0 };
} // namespace up
