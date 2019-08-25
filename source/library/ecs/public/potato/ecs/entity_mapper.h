// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/foundation/vector.h"

namespace up {
    /// Manages EntityIds and their mappings to archetypes and chunks
    ///
    class EntityMapper {
    public:
        auto allocate(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId;
        void recycle(EntityId entity) noexcept;

        auto tryParse(EntityId entity, ArchetypeId& out_archetype, uint16& out_chunk, uint16& out_index) const noexcept -> bool;

        void setArchetype(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept;
        void setIndex(EntityId entity, uint16 newChunk, uint16 newIndex) noexcept;

    private:
        static constexpr uint32 freeEntityIndex = static_cast<uint32>(-1);

        vector<uint64> _entityMapping;
        uint32 _freeEntityHead = freeEntityIndex;
    };
}
