// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include <potato/foundation/box.h>

namespace up {
    struct ComponentMeta;

    /// Describes the information about how components are laid out in an Archetype
    ///
    struct ChunkLayout {
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
    struct alignas(32) ChunkHeader {
        unsigned int count = 0;
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
} // namespace up
