// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include <potato/spud/box.h>
#include <potato/spud/vector.h>

namespace up {
    struct ComponentMeta;
    struct Chunk;
    class ArchetypeMapper;

    /// Describes the information about how components are laid out in an Archetype
    ///
    struct ChunkRowDesc {
        ComponentId component = ComponentId::Unknown;
        ComponentMeta const* meta = nullptr;
        uint16 offset = 0;
        uint16 width = 0;
    };

    /// Total size of a Chunk in bytes.
    ///
    static constexpr uint32 ChunkSizeBytes = 64 * 1024;

    /// The fixed header at the beginning of every Chunk
    ///
    struct alignas(64) ChunkHeader {
        ArchetypeId archetype = ArchetypeId::Empty;
        unsigned int entities = 0;
        unsigned int capacity = 0;
        Chunk* next = nullptr;
    };

    /// The payload (non-header data) of a Chunk.
    ///
    using ChunkPayload = char[ChunkSizeBytes - sizeof(ChunkHeader)];

    /// Chunks are the storage mechanism of Entities and their Components. A Chunk
    /// is allocated to an Archetype and will store a list of Components according
    /// to the Archetype's specified layout.
    ///
    struct Chunk {
        ChunkHeader header;
        ChunkPayload data;
    };

    static_assert(sizeof(up::Chunk) == up::ChunkSizeBytes, "Chunk has incorrect size; possibly unexpected member padding");

    /// Chunk allocator
    ///
    class ChunkAllocator {
    public:
        auto allocate(ArchetypeId archetype) -> Chunk*;
        void recycle(Chunk* chunk);

        auto chunks() const noexcept -> view<box<Chunk>> { return _chunks; }

    private:
        vector<box<Chunk>> _chunks;
        Chunk* _freeChunkHead = nullptr;
    };

    class ChunkMapper {
    public:
        auto chunks() const noexcept -> view<Chunk*> { return _chunks; }
        UP_ECS_API auto chunksOf(ArchetypeMapper const& mapper, ArchetypeId archetype) const noexcept -> view<Chunk*>;
        auto addChunk(ArchetypeMapper& mapper, ArchetypeId archetype, Chunk* chunk) -> int;
        void removeChunk(ArchetypeMapper& mapper, ArchetypeId archetype, int chunkIndex) noexcept;
        UP_ECS_API auto getChunk(ArchetypeMapper const& mapper, ArchetypeId archetype, int chunkIndex) const noexcept -> Chunk*;

    private:
        vector<Chunk*> _chunks;
    };
} // namespace up
