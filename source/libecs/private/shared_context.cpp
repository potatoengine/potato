// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include "potato/ecs/shared_context.h"
#include <potato/spud/find.h>
#include <potato/spud/sort.h>

auto up::EcsSharedContext::findComponentById(ComponentId id) const noexcept -> ComponentMeta const* {
    auto const* it = find(components, id, equality{}, &ComponentMeta::id);
    return it != components.end() ? it : nullptr;
}

auto up::EcsSharedContext::_findComponentByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const* {
    auto const* it = find(components, typeHash, equality{}, &ComponentMeta::typeHash);
    return it != components.end() ? it : nullptr;
}

auto up::EcsSharedContext::acquireChunk() -> Chunk* {
    if (freeChunkHead != nullptr) {
        Chunk* const chunk = freeChunkHead;
        freeChunkHead = chunk->header.next;
        chunk->header.next = nullptr;
        return chunk;
    }

    Chunk* const chunk = freeChunks.push_back(new_box<Chunk>()).get();
    return chunk;
}

void up::EcsSharedContext::recycleChunk(Chunk* chunk) noexcept {
    if (chunk == nullptr) {
        return;
    }

    chunk->header.archetype = ArchetypeId::Empty;
    chunk->header.entities = 0;
    chunk->header.next = freeChunkHead;
    freeChunkHead = chunk;
}

void up::EcsSharedContext::_bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept {
    UP_ASSERT(componentIds.size() == offsets.size());

    auto const layout = layoutOf(archetype);

    for (ComponentId const component : componentIds) {
        auto const desc = find(layout, component, {}, &ChunkRowDesc::component);
        UP_ASSERT(desc != layout.end());
        offsets.front() = desc->offset;
        offsets.pop_front();
    }
}

auto up::EcsSharedContext::_beginArchetype(bit_set components) -> ArchetypeId {
    // allocate a new archetype and associated id
    //
    auto id = ArchetypeId(archetypes.size());
    archetypes.push_back({static_cast<uint32>(layout.size())});
    archetypeMasks.push_back(std::move(components));

    return id;
}

auto up::EcsSharedContext::_finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId {
    auto& archData = archetypes[to_underlying(archetype)];
    auto& compSet = archetypeMasks[to_underlying(archetype)];
    archData.layoutLength = static_cast<uint16>(layout.size() - archData.layoutOffset);

    auto const newLayout = layout.subspan(archData.layoutOffset, archData.layoutLength);

    // sort rows by alignment for ideal packing
    //
    sort(newLayout, {}, [](const auto& row) noexcept { return row.meta->alignment; });

    // calculate total size of all components
    //
    size_t size = sizeof(EntityId);
    size_t padding = 0;
    for (auto const& row : newLayout) {
        padding += align_to(size, row.meta->alignment) - size;
        size += row.meta->size;
    }

    // calculate how many entities with this layout can fit in a single chunk
    //
    archData.maxEntitiesPerChunk = size != 0 ? static_cast<uint32>((sizeof(Chunk::Payload) - padding) / size) : 0;
    UP_ASSERT(archData.maxEntitiesPerChunk > 0);

    // calculate the row offets for the layout
    //
    size_t offset = sizeof(EntityId) * archData.maxEntitiesPerChunk;
    for (auto& row : newLayout) {
        offset = align_to(offset, row.meta->alignment);

        row.component = row.meta->id;
        row.offset = static_cast<uint32>(offset);
        row.width = row.meta->size;

        offset += row.width * archData.maxEntitiesPerChunk;
        UP_ASSERT(offset <= sizeof(Chunk::payload));

        compSet.set(row.meta->index);
    }

    // sort all rows by component id
    //
    sort(newLayout, {}, &ChunkRowDesc::component);

    return archetype;
}

auto up::EcsSharedContext::_findArchetype(bit_set const& set) noexcept -> FindResult {
    for (size_t index = 0; index != archetypes.size(); ++index) {
        if (archetypeMasks[index] == set) {
            return {true, static_cast<ArchetypeId>(index)};
        }
    }
    return {false, ArchetypeId::Empty};
}

auto up::EcsSharedContext::acquireArchetype(view<ComponentMeta const*> include, view<ComponentMeta const*> exclude) -> ArchetypeId {
    bit_set set;
    for (ComponentMeta const* meta : include) {
        set.set(meta->index);
    }
    for (ComponentMeta const* meta : exclude) {
        set.reset(meta->index);
    }

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set));

    // append all the other components
    //
    for (ComponentMeta const* meta : include) {
        layout.push_back({meta->id, meta});
    }

    return _finalizeArchetype(id);
}

auto up::EcsSharedContext::acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId {
    bit_set set = archetypeMasks[to_underlying(original)].clone();
    set.set(additional->index);

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set));
    auto const& originalArch = archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        auto const rowIndex = originalArch.layoutOffset + index;
        layout.push_back(layout[rowIndex]);
    }
    layout.push_back({additional->id, additional});

    return _finalizeArchetype(id);
}

auto up::EcsSharedContext::acquireArchetypeWithout(ArchetypeId original, ComponentMeta const* excluded) -> ArchetypeId {
    bit_set set = archetypeMasks[to_underlying(original)].clone();
    set.reset(excluded->index);

    if (auto [found, arch] = _findArchetype(set); found) {
        return arch;
    }

    auto id = _beginArchetype(std::move(set));
    auto const& originalArch = archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        auto const rowIndex = originalArch.layoutOffset + index;
        auto const& row = layout[rowIndex];
        if (row.meta != excluded) {
            layout.push_back(row);
        }
    }
    return _finalizeArchetype(id);
}
