// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/chunk.h"

auto up::ChunkAllocator::allocate() -> Chunk* {
    if (_freeChunkHead != nullptr) {
        Chunk* const chunk = _freeChunkHead;
        _freeChunkHead = chunk->header.next;
        chunk->header.next = nullptr;
        return chunk;
    }

    _chunks.push_back(new_box<Chunk>());
    return _chunks.back().get();
}

void up::ChunkAllocator::recycle(Chunk* chunk) {
    if (chunk == nullptr) {
        return;
    }

    chunk->header.next = _freeChunkHead;
    _freeChunkHead = chunk;
}
