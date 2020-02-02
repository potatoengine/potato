// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "potato/spud/vector.h"

namespace up {
    /// Manages EntityIds and their mappings to archetypes and chunks
    ///
    class EntityMapper {
    public:
        struct ParseLocation {
            ArchetypeId archetype = ArchetypeId::Empty;
            uint16 chunk = 0;
            uint16 index = 0;
        };

        struct TryParseLocation {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
            uint16 chunk = 0;
            uint16 index = 0;
        };

        auto allocate(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId;
        void recycle(EntityId entity) noexcept;

        auto isValid(EntityId) const noexcept -> bool;
        auto parse(EntityId entity) const noexcept -> ParseLocation;
        UP_ECS_API auto tryParse(EntityId entity) const noexcept -> TryParseLocation;

        void setArchetype(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept;
        void setIndex(EntityId entity, uint16 newChunk, uint16 newIndex) noexcept;

    private:
        static constexpr uint32 freeEntityIndex = static_cast<uint32>(-1);

        vector<uint64> _entityMapping;
        uint32 _freeEntityHead = freeEntityIndex;
    };
} // namespace up
