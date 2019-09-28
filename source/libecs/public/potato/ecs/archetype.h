// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    static constexpr uint32 ArchetypeComponentLimit = 256;

    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        ArchetypeId id = ArchetypeId::Unknown;
        vector<Chunk*> chunks;
        vector<ChunkRowDesc> chunkLayout;
        uint64 layoutHash = 0;
        uint32 maxEntitiesPerChunk = 0;
    };

    /// Manages the known list of Archetypes
    ///
    class ArchetypeMapper {
    public:
        using SelectSignature = void(ArchetypeId, view<int>);

        uint32 version() const noexcept { return _version;  }
        view<Archetype> archetypes() const noexcept { return _archetypes; }

        auto getArchetype(ArchetypeId arch) noexcept -> Archetype*;
        auto findArchetype(view<ComponentId> components) const noexcept -> Archetype const*;
        auto createArchetype(view<ComponentMeta const*> components) -> Archetype*;
        auto selectArchetypes(view<ComponentId> components, delegate_ref<SelectSignature> callback) const noexcept -> int;

    private:
        uint32 _version = 0;
        vector<Archetype> _archetypes;
    };
} // namespace up
