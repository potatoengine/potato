// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    static constexpr uint32 ArchetypeComponentLimit = 256;

    enum class ArchetypeLayoutId : uint64 {};

    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        ArchetypeId id = ArchetypeId::Unknown;
        mutable/*temporary*/ vector<Chunk*> chunks;
        vector<ChunkRowDesc> chunkLayout;
        ArchetypeLayoutId layoutHash = {};
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
        auto findArchetype(ArchetypeLayoutId layoutHash) const noexcept -> Archetype const*;
        auto createArchetype(view<ComponentMeta const*> components) -> Archetype const*;
        auto selectArchetypes(view<ComponentId> components, span<int> offsetsBuffer, delegate_ref<SelectSignature> callback) const noexcept -> int;

    private:
        uint32 _version = 0;
        vector<Archetype> _archetypes;
    };

    /// Hashes components for an Archetype
    ///
    class ArchetypeComponentHasher {
    public:
        using result_type = ArchetypeLayoutId;

        constexpr auto hash(ComponentId componentId) noexcept -> ArchetypeComponentHasher& {
            _state += static_cast<uint64>(componentId);
            return *this;
        }

        constexpr auto finalize() noexcept -> result_type { return static_cast<ArchetypeLayoutId>(_state); }

    private:
        static constexpr uint64 seed = 0;

        uint64 _state = seed;
    };
} // namespace up
