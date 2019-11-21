// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "component.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/utility.h"
#include "potato/ecs/common.h"
#include "potato/ecs/chunk.h"

namespace up {
    /// Archetypes are a common type of Entity with identical Component layout.
    ///
    struct Archetype {
        uint32 chunksOffset = 0;
        uint32 layoutOffset = 0;
        uint16 chunksLength = 0;
        uint16 layoutLength = 0;
        uint32 maxEntitiesPerChunk = 0;
    };

    /// Manages the known list of Archetypes
    ///
    class ArchetypeMapper {
    public:
        using SelectSignature = void(ArchetypeId, view<int>);

        ArchetypeMapper();

        auto version() const noexcept { return _version; }
        auto archetypes() const noexcept -> size_t { return _archetypes.size(); }
        auto layouts() const noexcept -> view<ChunkRowDesc> { return _layout; }
        auto chunks() const noexcept -> view<Chunk*> { return _chunks; }

        auto layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc> {
            auto const& arch = _archetypes[to_underlying(archetype)];
            return _layout.subspan(arch.layoutOffset, arch.layoutLength);
        }

        auto chunksOf(ArchetypeId archetype) const noexcept -> view<Chunk*> {
            auto const& arch = _archetypes[to_underlying(archetype)];
            return _chunks.subspan(arch.chunksOffset, arch.chunksLength);
        }

        auto acquireArchetype(view<ComponentMeta const*> components) -> ArchetypeId;
        auto acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId;
        auto acquireArchetypeWithout(ArchetypeId original, ComponentId excluded) -> ArchetypeId;

        template <typename... Components, typename Callback>
        auto selectArchetypes(size_t start, Callback&& callback) const noexcept -> size_t {
            ComponentId const components[sizeof...(Components)] = {getComponentId<Components>()...};
            int offsets[sizeof...(Components)];

            for (auto index = start; index < _archetypes.size(); ++index) {
                if (_matchArchetype(ArchetypeId(index), components, offsets)) {
                    callback(ArchetypeId(index), offsets);
                }
            }

            return _archetypes.size();
        }

        auto addChunk(ArchetypeId archetype, Chunk* chunk) -> int;
        void removeChunk(ArchetypeId archetype, int chunkIndex) noexcept;
        auto getChunk(ArchetypeId archetype, int chunkIndex) const noexcept -> Chunk* {
            return _chunks[_archetypes[to_underlying(archetype)].chunksOffset + chunkIndex];
        }

    private:
        template <typename P>
        auto _findArchetype(P&& predicate) noexcept -> Archetype*;
        UP_ECS_API auto _matchArchetype(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool;
        auto _beginArchetype() -> ArchetypeId;
        auto _finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId;

        uint32 _version = 0;
        vector<Chunk*> _chunks;
        vector<Archetype> _archetypes;
        vector<ChunkRowDesc> _layout;
    };
} // namespace up
