// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include <potato/spud/span.h>

namespace up {
    struct ComponentMeta;
    struct Chunk;

    /// Describes the information about how components are laid out in an Archetype
    ///
    struct ChunkRowDesc {
        ComponentId component = ComponentId::Unknown;
        ComponentMeta const* meta = nullptr;
        uint16 offset = 0;
        uint16 width = 0;
    };

    /// Chunks are the storage mechanism of Entities and their Components. A Chunk
    /// is allocated to an Archetype and will store a list of Components according
    /// to the Archetype's specified layout.
    ///
    struct Chunk {
        static constexpr uint32 SizeBytes = 64 * 1024;

        struct alignas(64) Header {
            ArchetypeId archetype = ArchetypeId::Empty;
            unsigned int entities = 0;
            unsigned int capacity = 0;
            Chunk* next = nullptr;
        } header;

        using Payload = char[SizeBytes - sizeof(Header)];
        Payload payload;

        auto entities() noexcept -> span<EntityId> {
            return {static_cast<EntityId*>(static_cast<void*>(payload)), header.entities};
        }

        auto entities() const noexcept -> view<EntityId> {
            return {static_cast<EntityId const*>(static_cast<void const*>(payload)), header.entities};
        }
    }; // namespace up

    static_assert(sizeof(Chunk) == Chunk::SizeBytes, "Chunk has incorrect size; possibly unexpected member padding");
} // namespace up
