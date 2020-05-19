// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "component.h"
#include "chunk.h"
#include "layout.h"
#include <potato/spud/int_types.h>
#include <potato/spud/span.h>
#include <potato/spud/vector.h>
#include <potato/spud/box.h>
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
        auto findComponentByType() const noexcept -> ComponentMeta const*;

        auto acquireChunk() -> Chunk*;
        void recycleChunk(Chunk* chunk) noexcept;

        inline auto layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow>;

        auto acquireArchetype(ArchetypeId original, view<ComponentMeta const*> include, view<ComponentMeta const*> exclude) -> ArchetypeId;

        UP_ECS_API auto _findComponentByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const*;
        UP_ECS_API auto _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool;

        vector<ComponentMeta> components;
        vector<ArchetypeLayout> archetypes = {ArchetypeLayout{0, 0, 0}};
        vector<LayoutRow> chunkRows;
        vector<box<Chunk>> allocatedChunks;
        Chunk* freeChunkHead = nullptr;
    };

    template <typename Component>
    auto EcsSharedContext::findComponentByType() const noexcept -> ComponentMeta const* {
        return _findComponentByTypeHash(typeid(Component).hash_code());
    }

    auto EcsSharedContext::layoutOf(ArchetypeId archetype) const noexcept -> view<LayoutRow> {
        auto const index = to_underlying(archetype);
        UP_ASSERT(index >= 0 && index < archetypes.size());
        auto const& arch = archetypes[to_underlying(archetype)];
        return chunkRows.subspan(arch.layoutOffset, arch.layoutLength);
    }

} // namespace up
