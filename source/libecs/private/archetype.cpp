// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/component.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include "potato/spud/find.h"
#include "potato/spud/bit_set.h"
#include "potato/runtime/assertion.h"

up::ArchetypeMapper::ArchetypeMapper() {
    // Archetype 0 is the empty archetype
    //
    auto id = _beginArchetype({}, 0);
    UP_ASSERT(id == ArchetypeId::Empty);
    _finalizeArchetype(id);
}

void up::ArchetypeMapper::_bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept {
    UP_ASSERT(componentIds.size() == offsets.size());

    auto const layout = layoutOf(archetype);

    for (ComponentId const component : componentIds) {
        auto const desc = find(layout, component, {}, &ChunkRowDesc::component);
        UP_ASSERT(desc != layout.end());
        offsets.front() = desc->offset;
        offsets.pop_front();
    }
}

auto up::ArchetypeMapper::_beginArchetype(bit_set components, uint32 chunkOffset) -> ArchetypeId {
    // allocate a new archetype and associated id
    //
    auto id = ArchetypeId(_archetypes.size());
    _archetypes.push_back({chunkOffset, static_cast<uint32>(_layout.size())});
    _components.push_back(std::move(components));

    return id;
}

auto up::ArchetypeMapper::_finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId {
    auto& archData = _archetypes[to_underlying(archetype)];
    auto& compSet = _components[to_underlying(archetype)];
    archData.layoutLength = static_cast<uint16>(_layout.size() - archData.layoutOffset);
    auto const layout = _layout.subspan(archData.layoutOffset, archData.layoutLength);

    // sort rows by alignment for ideal packing
    //
    up::sort(layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

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
    archData.maxEntitiesPerChunk = size != 0 ? static_cast<uint32>((sizeof(ChunkPayload) - padding) / size) : 0;
    UP_ASSERT(archData.maxEntitiesPerChunk > 0);

    // calculate the row offets for the layout
    //
    size_t offset = sizeof(EntityId) * archData.maxEntitiesPerChunk;
    for (auto& row : layout) {
        offset = align_to(offset, row.meta->alignment);
        row.offset = static_cast<uint32>(offset);
        offset += row.width * archData.maxEntitiesPerChunk;
        UP_ASSERT(offset <= sizeof(Chunk::data));

        compSet.set(row.meta->index);
    }

    // sort all rows by component id
    //
    up::sort(layout, {}, &ChunkRowDesc::component);

    return archetype;
}

auto up::ArchetypeMapper::_findArchetype(bit_set const& set) noexcept -> FindResult {
    for (size_t index = 0; index != _archetypes.size(); ++index) {
        if (_components[index] == set) {
            return {true, static_cast<ArchetypeId>(index)};
        }
    }
    return {false, ArchetypeId::Empty};
}

auto up::ArchetypeMapper::acquireArchetype(view<ComponentMeta const*> components, uint32 chunkOffset) -> ArchetypeId {
    bit_set set;
    for (ComponentMeta const* meta : components) {
        set.set(meta->index);
    }

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set), chunkOffset);

    // append all the other components
    //
    for (ComponentMeta const* meta : components) {
        _layout.push_back({meta->id, meta, 0, static_cast<uint16>(meta->size)});
    }

    return _finalizeArchetype(id);
}

auto up::ArchetypeMapper::acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional, uint32 chunkOffset) -> ArchetypeId {
    bit_set set = _components[to_underlying(original)].clone();
    set.set(additional->index);

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set), chunkOffset);
    auto const& originalArch = _archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        _layout.push_back(_layout[originalArch.layoutOffset + index]);
    }
    _layout.push_back({additional->id, additional, 0, static_cast<uint16>(additional->size)});

    return _finalizeArchetype(id);
}

auto up::ArchetypeMapper::acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded, uint32 chunkOffset) -> ArchetypeId {
    bit_set set = _components[to_underlying(original)].clone();
    set.reset(excluded->index);

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set), chunkOffset);
    auto const& originalArch = _archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        auto const& row = _layout[originalArch.layoutOffset + index];
        if (row.meta != excluded) {
            _layout.push_back(row);
        }
    }
    return _finalizeArchetype(id);
}
