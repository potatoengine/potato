// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/ecs/shared_context.h"
#include <potato/spud/find.h>
#include <potato/spud/sort.h>
#include <potato/spud/utility.h>

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

    Chunk* const chunk = allocatedChunks.push_back(new_box<Chunk>()).get();
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

auto up::EcsSharedContext::_bindArchetypeOffets(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool {
    UP_ASSERT(componentIds.size() == offsets.size());

    auto const layout = layoutOf(archetype);

    if (layout.size() < componentIds.size()) {
        return false;
    }

    for (ComponentId const component : componentIds) {
        auto const desc = find(layout, component, {}, &LayoutRow::component);
        if (desc == layout.end()) {
            return false;
        }
        offsets.front() = desc->offset;
        offsets.pop_front();
    }

    return true;
}

auto up::EcsSharedContext::acquireArchetype(ArchetypeId original, view<ComponentMeta const*> include, view<ComponentMeta const*> exclude) -> ArchetypeId {
    // attempt to find any existing archetype with the same component set
    //
    {
        auto const originalLayout = layoutOf(original);
        for (size_t index = 0; index != archetypes.size(); ++index) {
            auto const layout = layoutOf(ArchetypeId(index));
            if (any(exclude, [&layout](auto const* meta) noexcept { return contains(layout, meta, {}, &LayoutRow::meta); })) {
                continue;
            }
            if (!all(include, [&layout](auto const* meta) noexcept { return contains(layout, meta, {}, &LayoutRow::meta); })) {
                continue;
            }
            if (!all(originalLayout, [&layout](auto const& row) noexcept { return contains(layout, row.meta, {}, &LayoutRow::meta); })) {
                continue;
            }
            return ArchetypeId(index);
        }
    }

    // allocate a new Archetype entry
    auto id = ArchetypeId(archetypes.size());
    auto& archData = archetypes.push_back({static_cast<uint32>(chunkRows.size())});

    // append component from the original archetype
    //
    // NOTE: be careful since we're pushing into the container that stores the
    // original layout, so we can't safely use a span here!
    //
    auto const& originalArch = archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        auto const rowIndex = originalArch.layoutOffset + index;
        auto const& row = chunkRows[rowIndex];
        if (!contains(exclude, row.meta)) {
            chunkRows.push_back(chunkRows[rowIndex]);
        }
    }

    // append all the new components
    //
    for (ComponentMeta const* meta : include) {
        chunkRows.push_back({meta->id, meta});
    }

    // now that all components are added, we can calculate the total number of them
    // excluding all the duplicates
    //
    archData.layoutLength = static_cast<uint16>(chunkRows.size() - archData.layoutOffset);
    auto const newLayout = chunkRows.subspan(archData.layoutOffset, archData.layoutLength);

    // sort rows by alignment for ideal packing
    //
    sort(newLayout, {}, [](const auto& row) noexcept { return row.meta->alignment; });

    // calculate total size of all components including padding, used to determine how many
    // entities we can store in a chunk for this archetype
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

    // calculate the chunk offsets for each row of components in a chunk
    //
    size_t offset = sizeof(EntityId) * archData.maxEntitiesPerChunk;
    for (auto& row : newLayout) {
        offset = align_to(offset, row.meta->alignment);

        row.component = row.meta->id;
        row.offset = static_cast<uint32>(offset);
        row.width = row.meta->size;

        offset += row.width * archData.maxEntitiesPerChunk;
        UP_ASSERT(offset <= sizeof(Chunk::payload));
    }

    // sort all rows by component id in the new layout, so we can use binary search for
    // component id lookups
    //
    sort(newLayout, {}, &LayoutRow::component);

    return id;
}
