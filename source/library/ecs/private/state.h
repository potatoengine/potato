// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/vector.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"
#include "potato/ecs/entity.h"
#include "potato/ecs/archetype.h"
#include "chunk.h"

namespace up {
    struct State {
        vector<EntityMapping> entityMapping;
        vector<rc<Archetype>> archetypes;
        vector<uint32> entityFreeList;
        vector<box<Chunk>> chunkPool;
    };
}
