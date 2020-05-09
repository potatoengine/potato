// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "../component.h"
#include "../chunk.h"
#include <potato/spud/int_types.h>
#include <potato/spud/span.h>
#include <potato/spud/vector.h>
#include <potato/spud/find.h>
#include <potato/spud/box.h>
#include <potato/spud/sort.h>
#include <potato/spud/bit_set.h>

namespace up::_detail {
    struct EcsContext {
        struct ArchetypeLayout {
            uint32 layoutOffset = 0;
            uint16 layoutLength = 0;
            uint16 maxEntitiesPerChunk = 0;
        };

        struct FindResult {
            bool success = false;
            ArchetypeId archetype = ArchetypeId::Empty;
        };

        inline auto findById(ComponentId id) const noexcept -> ComponentMeta const*;
        inline auto findByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const*;

        template <typename Component>
        auto findByType() const noexcept -> ComponentMeta const* { return findByTypeHash(typeid(Component).hash_code()); }

        inline auto allocate(ArchetypeId archetype) -> Chunk*;
        inline void recycle(Chunk* chunk) noexcept;

        auto layoutOf(ArchetypeId archetype) const noexcept -> view<ChunkRowDesc> {
            auto const& arch = _archetypes[to_underlying(archetype)];
            return _layout.subspan(arch.layoutOffset, arch.layoutLength);
        }

        auto getEntitiesPerChunk(ArchetypeId archetype) const noexcept -> uint16 {
            auto const index = to_underlying(archetype);
            return index >= 0 && index < _archetypes.size() ? _archetypes[index].maxEntitiesPerChunk : 0;
        }

        inline auto acquireArchetype(view<ComponentMeta const*> components) -> ArchetypeId;
        inline auto acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId;
        inline auto acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded) -> ArchetypeId;

        template <size_t ComponentCount, typename Callback>
        auto selectArchetypes(size_t start, bit_set const& mask, ComponentId const (&components)[ComponentCount], Callback&& callback) const noexcept -> size_t {
            int offsets[ComponentCount];

            for (auto index = start; index < _archetypes.size(); ++index) {
                if (_archetypeMasks[index].has_all(mask)) {
                    _bindArchetypeOffets(ArchetypeId(index), components, offsets);
                    callback(ArchetypeId(index), offsets);
                }
            }

            return _archetypes.size();
        }

        inline auto _findArchetype(bit_set const& set) noexcept -> FindResult;
        inline void _bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept;
        inline auto _beginArchetype(bit_set components) -> ArchetypeId;
        inline auto _finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId;

        vector<ComponentMeta> components;
        vector<ArchetypeLayout> _archetypes;
        vector<bit_set> _archetypeMasks;
        vector<ChunkRowDesc> _layout;
        vector<box<Chunk>> _chunks;
        Chunk* _freeChunkHead = nullptr;
    };

    auto EcsContext::findById(ComponentId id) const noexcept -> ComponentMeta const* {
        auto const* it = find(components, id, equality{}, &ComponentMeta::id);
        return it != components.end() ? it : nullptr;
    }

    auto EcsContext::findByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const* {
        auto const* it = find(components, typeHash, equality{}, &ComponentMeta::typeHash);
        return it != components.end() ? it : nullptr;
    }

    auto EcsContext::allocate(ArchetypeId archetype) -> Chunk* {
        if (_freeChunkHead != nullptr) {
            Chunk* const chunk = _freeChunkHead;
            _freeChunkHead = chunk->header.next;
            chunk->header.archetype = archetype;
            chunk->header.next = nullptr;
            return chunk;
        }

        _chunks.push_back(new_box<Chunk>());
        Chunk* const chunk = _chunks.back().get();
        chunk->header.archetype = archetype;
        return chunk;
    }

    void EcsContext::recycle(Chunk* chunk) noexcept {
        if (chunk == nullptr) {
            return;
        }

        chunk->header.archetype = ArchetypeId::Empty;
        chunk->header.entities = 0;
        chunk->header.next = _freeChunkHead;
        _freeChunkHead = chunk;
    }

    void EcsContext::_bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept {
        UP_ASSERT(componentIds.size() == offsets.size());

        auto const layout = layoutOf(archetype);

        for (ComponentId const component : componentIds) {
            auto const desc = find(layout, component, {}, &ChunkRowDesc::component);
            UP_ASSERT(desc != layout.end());
            offsets.front() = desc->offset;
            offsets.pop_front();
        }
    }

    auto EcsContext::_beginArchetype(bit_set components) -> ArchetypeId {
        // allocate a new archetype and associated id
        //
        auto id = ArchetypeId(_archetypes.size());
        _archetypes.push_back({static_cast<uint32>(_layout.size())});
        _archetypeMasks.push_back(std::move(components));

        return id;
    }

    auto EcsContext::_finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId {
        auto& archData = _archetypes[to_underlying(archetype)];
        auto& compSet = _archetypeMasks[to_underlying(archetype)];
        archData.layoutLength = static_cast<uint16>(_layout.size() - archData.layoutOffset);
        auto const layout = _layout.subspan(archData.layoutOffset, archData.layoutLength);

        // sort rows by alignment for ideal packing
        //
        sort(layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

        // calculate total size of all components
        //
        size_t size = sizeof(EntityId);
        size_t padding = 0;
        for (auto const& row : layout) {
            padding += align_to(size, row.meta->alignment) - size;
            size += row.width;
        }

        // calculate how many entities with this layout can fit in a single chunk
        //
        archData.maxEntitiesPerChunk = size != 0 ? static_cast<uint32>((sizeof(Chunk::Payload) - padding) / size) : 0;
        UP_ASSERT(archData.maxEntitiesPerChunk > 0);

        // calculate the row offets for the layout
        //
        size_t offset = sizeof(EntityId) * archData.maxEntitiesPerChunk;
        for (auto& row : layout) {
            offset = align_to(offset, row.meta->alignment);
            row.offset = static_cast<uint32>(offset);
            offset += row.width * archData.maxEntitiesPerChunk;
            UP_ASSERT(offset <= sizeof(Chunk::payload));

            compSet.set(row.meta->index);
        }

        // sort all rows by component id
        //
        sort(layout, {}, &ChunkRowDesc::component);

        return archetype;
    }

    auto EcsContext::_findArchetype(bit_set const& set) noexcept -> FindResult {
        for (size_t index = 0; index != _archetypes.size(); ++index) {
            if (_archetypeMasks[index] == set) {
                return {true, static_cast<ArchetypeId>(index)};
            }
        }
        return {false, ArchetypeId::Empty};
    }

    auto EcsContext::acquireArchetype(view<ComponentMeta const*> components) -> ArchetypeId {
        bit_set set;
        for (ComponentMeta const* meta : components) {
            set.set(meta->index);
        }

        if (auto [found, arch] = _findArchetype(set); found) {
            return arch;
        }

        auto id = _beginArchetype(std::move(set));

        // append all the other components
        //
        for (ComponentMeta const* meta : components) {
            _layout.push_back({meta->id, meta, 0, static_cast<uint16>(meta->size)});
        }

        return _finalizeArchetype(id);
    }

    auto EcsContext::acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId {
        bit_set set = _archetypeMasks[to_underlying(original)].clone();
        set.set(additional->index);

        if (auto [found, arch] = _findArchetype(set); found) {
            return arch;
        }

        auto id = _beginArchetype(std::move(set));
        auto const& originalArch = _archetypes[to_underlying(original)];
        for (int index = 0; index != originalArch.layoutLength; ++index) {
            _layout.push_back(_layout[originalArch.layoutOffset + index]);
        }
        _layout.push_back({additional->id, additional, 0, static_cast<uint16>(additional->size)});

        return _finalizeArchetype(id);
    }

    auto EcsContext::acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded) -> ArchetypeId {
        bit_set set = _archetypeMasks[to_underlying(original)].clone();
        set.reset(excluded->index);

        if (auto [found, arch] = _findArchetype(set); found) {
            return arch;
        }

        auto id = _beginArchetype(std::move(set));
        auto const& originalArch = _archetypes[to_underlying(original)];
        for (int index = 0; index != originalArch.layoutLength; ++index) {
            auto const& row = _layout[originalArch.layoutOffset + index];
            if (row.meta != excluded) {
                _layout.push_back(row);
            }
        }
        return _finalizeArchetype(id);
    }

} // namespace up::_detail
