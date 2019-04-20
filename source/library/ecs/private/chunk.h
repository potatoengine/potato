// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/vector.h"

namespace up {
    struct alignas(32) ChunkHeader {
        int count = 0;
    };

    struct Chunk {
        static constexpr uint32 allocatedSize = 64 * 1024;
        static constexpr uint32 dataSize = allocatedSize - sizeof(ChunkHeader);

        ChunkHeader header;
        char data[dataSize];
    };


    static_assert(sizeof(Chunk) == Chunk::allocatedSize);
}
