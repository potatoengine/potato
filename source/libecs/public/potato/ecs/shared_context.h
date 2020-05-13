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

        auto findComponentById(ComponentId id) const noexcept -> ComponentMeta const*;

        template <typename Component>
        auto findComponentByType() const noexcept -> ComponentMeta const* { return _findComponentByTypeHash(typeid(Component).hash_code()); }

        auto acquireChunk() -> Chunk*;
        void recycleChunk(Chunk* chunk) noexcept;

        auto layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc> {
            auto const& arch = archetypes[to_underlying(archetype)];
            return layout.subspan(arch.layoutOffset, arch.layoutLength);
        }

        auto acquireArchetype(view<ComponentMeta const*> include, view<ComponentMeta const*> exclude) -> ArchetypeId;
        auto acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId;
        auto acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded) -> ArchetypeId;

        template <size_t ComponentCount, typename Callback>
        auto selectArchetypes(size_t start, bit_set const& mask, ComponentId const (&components)[ComponentCount], Callback&& callback) const noexcept -> size_t {
            int offsets[ComponentCount];

            for (auto index = start; index < archetypes.size(); ++index) {
                if (archetypeMasks[index].has_all(mask)) {
                    _bindArchetypeOffets(ArchetypeId(index), components, offsets);
                    callback(ArchetypeId(index), offsets);
                }
            }

            return archetypes.size();
        }

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

} // namespace up
