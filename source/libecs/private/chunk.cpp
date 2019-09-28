// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/chunk.h"

auto up::ChunkAllocator::allocate(ArchetypeId archetype) -> Chunk* {
    if (_freeChunkHead != nullptr) {
        Chunk* const chunk = _freeChunkHead;
        _freeChunkHead = chunk->header.next;
        chunk->header.archetype = archetype;
        chunk->header.next = nullptr;
        return chunk;
    }

    _chunks.push_back(new_box<Chunk>());
    Chunk* const chunk = _chunks.back().get();
    chunk->header.archetype = archetype;
    return chunk;
}

void up::ChunkAllocator::recycle(Chunk* chunk) {
    if (chunk == nullptr) {
        return;
    }

    chunk->header.archetype = ArchetypeId::Unknown;
    chunk->header.entities = 0;
    chunk->header.next = _freeChunkHead;
    _freeChunkHead = chunk;
}
