// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/chunk.h"
#include "potato/ecs/archetype.h"

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

    chunk->header.archetype = ArchetypeId::Empty;
    chunk->header.entities = 0;
    chunk->header.next = _freeChunkHead;
    _freeChunkHead = chunk;
}

auto up::ChunkMapper::chunksOf(ArchetypeMapper const& mapper, ArchetypeId arch) const noexcept -> view<Chunk*> {
    auto const* desc = mapper.getArchetype(arch);
    if (desc != nullptr) {
        return _chunks.subspan(desc->chunksOffset, desc->chunksLength);
    }
    return {};
}

auto up::ChunkMapper::addChunk(ArchetypeMapper& mapper, ArchetypeId arch, Chunk* chunk) -> int {
    UP_ASSERT(arch == chunk->header.archetype);

    Archetype* archData = mapper.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    int const lastIndex = archData->chunksLength;

    chunk->header.capacity = archData->maxEntitiesPerChunk;

    _chunks.insert(_chunks.begin() + archData->chunksOffset + archData->chunksLength, chunk);
    ++archData->chunksLength;

    for (auto& updateArch : mapper.archetypes().subspan(to_underlying(arch) + 1)) {
        ++updateArch.chunksOffset;
    }

    return lastIndex;
}

void up::ChunkMapper::removeChunk(ArchetypeMapper& mapper, ArchetypeId arch, int chunkIndex) noexcept {
    Archetype* archData = mapper.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    _chunks.erase(_chunks.begin() + archData->chunksOffset + chunkIndex);
    --archData->chunksLength;

    for (auto& updateArch : mapper.archetypes().subspan(to_underlying(arch) + 1)) {
        --updateArch.chunksOffset;
    }
}

auto up::ChunkMapper::getChunk(ArchetypeMapper const& mapper, ArchetypeId arch, int chunkIndex) const noexcept -> Chunk* {
    Archetype const* archData = mapper.getArchetype(arch);
    return archData != nullptr ? _chunks[archData->chunksOffset + chunkIndex] : nullptr;
}
