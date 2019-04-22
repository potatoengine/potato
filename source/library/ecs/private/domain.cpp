// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/domain.h"

up::EntityDomain::~EntityDomain() = default;

auto up::EntityDomain::allocateEntityId() noexcept -> EntityId {
    if (_freeEntityHead == static_cast<decltype(_freeEntityHead)>(-1)) {
        // nothing in free list
        uint32 firstGeneration = 1;
        entityMapping.push_back({0, firstGeneration, 0});
        return makeEntityId(static_cast<uint32>(entityMapping.size()), firstGeneration);
    }

    uint32 index = _freeEntityHead;
    _freeEntityHead = entityMapping[index].index;
    return makeEntityId(index, ++entityMapping[index].generation);
}

void up::EntityDomain::returnEntityId(EntityId entity) noexcept {
    uint32 index = getEntityIndex(entity);

    entityMapping[index].archetype = 0;
    ++entityMapping[index].generation;
    entityMapping[index].index = _freeEntityHead;

    _freeEntityHead = index;
}

auto up::EntityDomain::allocateChunk() -> box<EntityChunk> {
    if (_chunkPool.empty()) {
        return new_box<EntityChunk>();
    }

    box<EntityChunk> chunk = std::move(_chunkPool.back());
    _chunkPool.pop_back();
    return chunk;
}

void up::EntityDomain::returnChunk(box<EntityChunk> chunk) noexcept {
    _chunkPool.push_back(std::move(chunk));
}
