// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"
#include "potato/ecs/entity.h"
#include "potato/ecs/archetype.h"

namespace up {
    struct EntityChunk {
        static constexpr uint32 size = 64 * 1024;

        struct alignas(32) Header {
            int count = 0;
        };
        using Payload = char[size - sizeof(Header)];

        Header header;
        Payload data;
    };
    static_assert(sizeof(EntityChunk) == EntityChunk::size);

    struct EntityMapping {
        uint32 generation = 0;
        uint32 archetype = 0;
        uint32 index = 0;
    };
} // namespace up
