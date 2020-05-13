// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "component.h"
#include "chunk.h"
#include <potato/spud/int_types.h>
#include <potato/spud/span.h>
#include <potato/spud/vector.h>
#include <potato/spud/box.h>
#include <potato/spud/bit_set.h>
#include <potato/spud/rc.h>

namespace up {
    struct EcsSharedContext : shared<EcsSharedContext> {
        struct ArchetypeLayout {
            uint32 layoutOffset = 0;
            uint16 layoutLength = 0;
            uint16 maxEntitiesPerChunk = 0;
        };

        struct FindResult {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
        };

        EcsSharedContext();

        auto findComponentById(ComponentId id) const noexcept -> ComponentMeta const*;

        template <typename Component>
        auto findComponentByType() const noexcept -> ComponentMeta const*;

        auto acquireChunk() -> Chunk*;
        void recycleChunk(Chunk* chunk) noexcept;

        inline auto layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc>;

        auto acquireArchetype(ArchetypeId original, view<ComponentMeta const*> include, view<ComponentMeta const*> exclude) -> ArchetypeId;

        template <size_t ComponentCount, typename Callback>
        auto selectArchetypes(size_t start, bit_set const& mask, ComponentId const (&components)[ComponentCount], Callback&& callback) const noexcept -> size_t
            requires is_invocable_v<Callback, ArchetypeId, int (&)[ComponentCount]>;

        UP_ECS_API auto _findComponentByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const*;

        UP_ECS_API void _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept;
        auto _findArchetype(bit_set const& set) noexcept -> FindResult;
        auto _beginArchetype(bit_set components) -> ArchetypeId;
        auto _finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId;

        vector<ComponentMeta> components;
        vector<ArchetypeLayout> archetypes;
        vector<bit_set> archetypeMasks;
        vector<ChunkRowDesc> layout;
        vector<box<Chunk>> freeChunks;
        Chunk* freeChunkHead = nullptr;
    };

    template <typename Component>
    auto EcsSharedContext::findComponentByType() const noexcept -> ComponentMeta const* {
        return _findComponentByTypeHash(typeid(Component).hash_code());
    }

    auto EcsSharedContext::layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc> {
        auto const index = to_underlying(archetype);
        UP_ASSERT(index >= 0 && index < archetypes.size());
        auto const& arch = archetypes[to_underlying(archetype)];
        return layout.subspan(arch.layoutOffset, arch.layoutLength);
    }

    template <size_t ComponentCount, typename Callback>
    auto EcsSharedContext::selectArchetypes(size_t start, bit_set const& mask, ComponentId const (&components)[ComponentCount], Callback&& callback) const noexcept -> size_t
        requires is_invocable_v<Callback, ArchetypeId, int (&)[ComponentCount]> {
        int offsets[ComponentCount];

        for (auto index = start; index < archetypes.size(); ++index) {
            if (archetypeMasks[index].has_all(mask)) {
                _bindArchetypeOffets(ArchetypeId(index), components, offsets);
                callback(ArchetypeId(index), offsets);
            }
        }

        return archetypes.size();
    }

} // namespace up
