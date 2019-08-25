// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

/// Identififies the location of an Entity within the Archetype/Chunk storage
///
struct up::World::Location {
    Archetype* archetype = nullptr;
    Chunk* chunk = nullptr;
    uint16 chunkIndex = 0;
    uint16 subIndex = 0;
};
