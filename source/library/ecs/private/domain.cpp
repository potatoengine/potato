// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/domain.h"

up::EntityDomain::~EntityDomain() = default;

auto up::EntityDomain::allocateEntityId() noexcept -> EntityId {
    if (_freeEntityHead == static_cast<decltype(_freeEntityHead)>(-1)) {
        // nothing in free list
        return makeEntityId(static_cast<uint32>(entityMapping.size()), 1);
    }

    uint32 index = _freeEntityHead;
    _freeEntityHead = entityMapping[index].index;
    return makeEntityId(index, ++entityMapping[index].generation);
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
