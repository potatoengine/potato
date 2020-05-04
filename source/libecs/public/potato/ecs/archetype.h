// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "component.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/utility.h"
#include "potato/spud/bit_set.h"
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

        auto archetypeIds() const noexcept -> sequence<ArchetypeId> {
            return {ArchetypeId::Empty, static_cast<ArchetypeId>(_archetypes.size())};
        }

        auto archetypes() const noexcept -> view<Archetype> { return _archetypes; }
        auto archetypes() noexcept -> span<Archetype> { return _archetypes; }

        auto layouts() const noexcept -> view<ChunkRowDesc> { return _layout; }

        auto layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc> {
            auto const& arch = _archetypes[to_underlying(archetype)];
            return _layout.subspan(arch.layoutOffset, arch.layoutLength);
        }

        auto getArchetype(ArchetypeId archetype) const noexcept -> Archetype const* {
            auto const index = to_underlying(archetype);
            return index >= 0 && index < _archetypes.size() ? &_archetypes[index] : nullptr;
        }

        auto getArchetype(ArchetypeId archetype) noexcept -> Archetype* {
            auto const index = to_underlying(archetype);
            return index >= 0 && index < _archetypes.size() ? &_archetypes[index] : nullptr;
        }

        auto acquireArchetype(view<ComponentMeta const*> components, uint32 chunkOffset) -> ArchetypeId;
        auto acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional, uint32 chunkOffset) -> ArchetypeId;
        auto acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded, uint32 chunkOffset) -> ArchetypeId;

        template <size_t ComponentCount, typename Callback>
        auto selectArchetypes(size_t start, bit_set const& mask, ComponentId const (&components)[ComponentCount], Callback&& callback) const noexcept -> size_t {
            int offsets[ComponentCount];

            for (auto index = start; index < _archetypes.size(); ++index) {
                if (_components[index].has_all(mask)) {
                    _bindArchetypeOffets(ArchetypeId(index), components, offsets);
                    callback(ArchetypeId(index), offsets);
                }
            }

            return _archetypes.size();
        }

    private:
        struct FindResult {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
        };
        auto _findArchetype(bit_set const& set) noexcept -> FindResult;
        UP_ECS_API void _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept;
        auto _beginArchetype(bit_set components, uint32 chunkOffset) -> ArchetypeId;
        auto _finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId;

        vector<Archetype> _archetypes;
        vector<bit_set> _components;
        vector<ChunkRowDesc> _layout;
    };
} // namespace up
