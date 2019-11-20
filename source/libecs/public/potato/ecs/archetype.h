// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/utility.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    static constexpr uint32 ArchetypeComponentLimit = 256;

    enum class ArchetypeLayoutId : uint64 {};

    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        ArchetypeId id = ArchetypeId::Unknown;
        uint32 chunksOffset = 0;
        uint32 layoutOffset = 0;
        uint16 chunksLength = 0;
        uint16 layoutLength = 0;
        ArchetypeLayoutId layoutHash = {};
        uint32 maxEntitiesPerChunk = 0;
    };

    /// Manages the known list of Archetypes
    ///
    class ArchetypeMapper {
    public:
        using SelectSignature = void(ArchetypeId, view<int>);

        uint32 version() const noexcept { return _version; }
        view<Archetype> archetypes() const noexcept { return _archetypes; }
        view<ChunkRowDesc> layouts() const noexcept { return _layout; }
        view<Chunk*> chunks() const noexcept { return _chunks; }

        /// Fetches a specified Archetype.
        ///
        auto getArchetype(ArchetypeId arch) noexcept -> Archetype* {
            auto const index = to_underlying(arch);
            UP_ASSERT(index >= 1 && index <= _archetypes.size());
            return &_archetypes[index - 1];
        }

        /// Fetch an Archetype by its layout hash.
        ///
        auto findArchetype(ArchetypeLayoutId layoutHash) noexcept -> Archetype* {
            for (Archetype& arch : _archetypes) {
                if (arch.layoutHash == layoutHash) {
                    return &arch;
                }
            }

            return nullptr;
        }

        auto createArchetype(view<ComponentMeta const*> components) -> Archetype*;

        UP_ECS_API auto selectArchetypes(view<ComponentId> components, span<int> offsetsBuffer, size_t start, delegate_ref<SelectSignature> callback) const noexcept -> size_t;

        UP_ECS_API auto addChunk(Archetype& arch, Chunk* chunk) -> size_t;
        UP_ECS_API void removeChunk(Archetype& arch, size_t chunkIndex) noexcept;

    private:
        auto _matchArchetype(Archetype const& arch, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool;
        void _calculateLayout(Archetype& archetype, view<ComponentMeta const*> components);

        uint32 _version = 0;
        vector<Chunk*> _chunks;
        vector<Archetype> _archetypes;
        vector<ChunkRowDesc> _layout;
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

        constexpr auto finalize() noexcept -> result_type {
            return static_cast<ArchetypeLayoutId>(_state);
        }

    private:
        static constexpr uint64 seed = 0;

        uint64 _state = seed;
    };
} // namespace up
