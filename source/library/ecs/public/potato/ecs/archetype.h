// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/box.h"
#include "potato/foundation/vector.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        vector<box<Chunk>> chunks;
        vector<ChunkLayout> chunkLayout;
        uint64 layoutHash = 0;
        uint32 entityCount = 0;
        uint32 maxEntitiesPerChunk = 0;

        /// Returns index of the Layout for a Component.
        ///
        /// @return index of component or -1 if no match.
        ///
        int32 indexOfLayout(ComponentId component) const noexcept {
            for (int32 i = 0; i != static_cast<int32>(chunkLayout.size()); ++i) {
                if (chunkLayout[i].component == component) {
                    return i;
                }
            }
            return -1;
        }
    };
} // namespace up
