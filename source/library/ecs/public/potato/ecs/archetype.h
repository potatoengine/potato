// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/vector.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        vector<Chunk*> chunks;
        vector<ChunkLayout> chunkLayout;
        uint64 layoutHash = 0;
        uint32 entityCount = 0;
        uint32 maxEntitiesPerChunk = 0;
    };
} // namespace up
