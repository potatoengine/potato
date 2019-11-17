// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include "potato/spud/find.h"
#include "potato/runtime/assertion.h"

namespace up {
    static auto matchArchetype(Archetype const& arch, view<ComponentId> componentIds, span<int> offsets) noexcept -> bool {
        UP_ASSERT(componentIds.size() == offsets.size());

        for (ComponentId const component : componentIds) {
            auto const layout = find(arch.chunkLayout, component, {}, &ChunkRowDesc::component);
            if (layout == arch.chunkLayout.end()) {
                return false;
            }
            offsets.front() = layout->offset;
            offsets.pop_front();
        }
        return true;
    };

    static void calculateLayout(Archetype& archetype, view<ComponentMeta const*> components) {
        archetype.chunkLayout.reserve(components.size() + 1);

        // we'll always include the EntityId in the layout
        //
        auto const entityMeta = ComponentMeta::get<Entity>();
        archetype.chunkLayout.push_back({entityMeta->id, entityMeta, 0, static_cast<uint16>(entityMeta->size)});
        archetype.chunkLayout[0].meta = entityMeta;

        // append all the other components
        //
        for (ComponentMeta const* meta : components) {
            archetype.chunkLayout.push_back({meta->id, meta, 0, static_cast<uint16>(meta->size)});
        }

        // sort rows by alignment for ideal packing
        //
        up::sort(
            archetype.chunkLayout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

        // calculate total size of all components
        //
        size_t size = 0;
        size_t padding = 0;
        for (auto const& row : archetype.chunkLayout) {
            padding += align_to(size, row.meta->alignment) - size;
            size += row.width;
        }

        // calculate how many entities with this layout can fit in a single chunk
        //
        archetype.maxEntitiesPerChunk = static_cast<uint32>((sizeof(ChunkPayload) - padding) / size);
        UP_ASSERT(archetype.maxEntitiesPerChunk > 0);

        // calculate the row offets for the layout
        //
        size_t offset = 0;
        for (auto& row : archetype.chunkLayout) {
            offset = align_to(offset, row.meta->alignment);
            row.offset = static_cast<uint32>(offset);
            offset += row.width * archetype.maxEntitiesPerChunk;
            UP_ASSERT(offset <= sizeof(Chunk::data));
        }
    }
} // namespace up

auto up::ArchetypeMapper::createArchetype(view<ComponentMeta const*> components) -> Archetype const* {
    ArchetypeComponentHasher hasher;
    for (auto meta : components) {
        hasher.hash(meta->id);
    }
    auto const hash = hasher.finalize();

    UP_ASSERT(findArchetype(hash) == nullptr);

    // bump so Query objects know that the list of archetypes has changed
    //
    ++_version;

    auto arch = Archetype{};
    arch.id = ArchetypeId(_archetypes.size() + 1);
    arch.layoutHash = hash;
    calculateLayout(arch, components);

    _archetypes.push_back(std::move(arch));

    return &_archetypes.back();
}

auto up::ArchetypeMapper::selectArchetypes(view<ComponentId> componentIds, span<int> offsetsBuffer, delegate_ref<SelectSignature> callback) const noexcept -> int {
    int matches = 0;

    for (Archetype const& arch : _archetypes) {
        if (matchArchetype(arch, componentIds, offsetsBuffer)) {
            ++matches;
            callback(arch.id, offsetsBuffer.first(componentIds.size()));
        }
    }

    return matches;
}
