// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/component.h"
#include "potato/foundation/utility.h"
#include "potato/foundation/sort.h"
#include "potato/foundation/find.h"
#include "potato/runtime/assertion.h"

namespace up {
    template <typename Type, typename Projection = up::identity>
    static constexpr auto hashComponents(span<Type> input, Projection const& proj = {}, uint64 seed = 0) noexcept -> uint64 {
        for (Type const& value : input) {
            seed ^= static_cast<up::uint64>(project(proj, value));
        }
        return seed;
    }

    static constexpr auto matchArchetype(Archetype const& arch, view<ComponentId> componentIds, int* offsets) noexcept -> bool {
        for (ComponentId const component : componentIds) {
            auto const layout = find(arch.chunkLayout, component, {}, &ChunkRowDesc::component);
            if (layout == arch.chunkLayout.end()) {
                return false;
            }
            *offsets++ = layout->offset;
        }
        return true;
    };
} // namespace up

auto up::ArchetypeMapper::getArchetype(ArchetypeId arch) const noexcept -> Archetype* {
    auto const index = to_underlying(arch);
    UP_ASSERT(index >= 1 && index <= _archetypes.size());
    return _archetypes[index - 1].get();
}

auto up::ArchetypeMapper::findArchetype(view<ComponentId> components) const noexcept -> Archetype const* {
    uint64 const hash = hashComponents(components);

    for (box<Archetype> const& arch : _archetypes) {
        if (arch->layoutHash == hash) {
            return arch.get();
        }
    }

    return nullptr;
}

auto up::ArchetypeMapper::createArchetype(view<ComponentMeta const*> components) -> Archetype* {
    uint64 const hash = hashComponents(components, &ComponentMeta::id);

    for (box<Archetype> const& arch : _archetypes) {
        if (arch->layoutHash == hash) {
            return arch.get();
        }
    }

    // bump so Query objects know that the list of archetypes has changed
    //
    ++_version;

    _archetypes.push_back(new_box<Archetype>());
    Archetype& arch = *_archetypes.back().get();
    arch.id = ArchetypeId(_archetypes.size());

    _populateArchetype(arch, components);

    UP_ASSERT(arch.layoutHash == hash);

    return &arch;
}

auto up::ArchetypeMapper::selectArchetypes(view<ComponentId> componentIds, delegate_ref<SelectSignature> callback) const noexcept -> int {
    int offsets[ArchetypeComponentLimit];
    int matches = 0;

    for (box<Archetype> const& arch : _archetypes) {
        auto const& archetype = *arch;

        if (matchArchetype(*arch, componentIds, offsets)) {
            ++matches;
            callback(archetype.id, span{offsets}.first(componentIds.size()));
        }
    }

    return matches;
}

void up::ArchetypeMapper::_populateArchetype(Archetype& archetype, view<ComponentMeta const*> components) noexcept {
    archetype.chunkLayout.resize(components.size());

    // we'll always include the EntityId is the layout, so include its size
    size_t size = sizeof(EntityId);

    for (size_t i = 0; i != components.size(); ++i) {
        ComponentMeta const& meta = *components[i];

        archetype.chunkLayout[i].component = meta.id;
        archetype.chunkLayout[i].meta = &meta;
        size = align_to(size, meta.alignment);
        size += meta.size;
    }
    UP_ASSERT(size <= sizeof(Chunk::data));

    if (size != 0) {
        _calculateLayout(archetype, size);
    }

    // Required for partial match algorithm
    sort(archetype.chunkLayout, {}, &ChunkRowDesc::component);
}

void up::ArchetypeMapper::_calculateLayout(Archetype& archetype, size_t size) noexcept {
    // calculate layout hash
    archetype.layoutHash = hashComponents(span{archetype.chunkLayout}, &ChunkRowDesc::component);

    // assign pointer offers by alignment
    up::sort(
        archetype.chunkLayout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

    // FIXME: figure out why we need this size + 1, the arithmetic surprises me
    archetype.maxEntitiesPerChunk = static_cast<uint32>(sizeof(ChunkPayload) / (size + 1));

    // we'll always include the EntityId is the layout, so include it as offset
    size_t offset = sizeof(EntityId) * archetype.maxEntitiesPerChunk;

    for (size_t i = 0; i != archetype.chunkLayout.size(); ++i) {
        ComponentMeta const& meta = *archetype.chunkLayout[i].meta;

        // align as required (requires alignment to be a power of 2)
        UP_ASSERT((meta.alignment & (meta.alignment - 1)) == 0);
        offset = align_to(offset, meta.alignment);
        UP_ASSERT(offset + archetype.maxEntitiesPerChunk * meta.size + meta.size <= sizeof(ChunkPayload));

        archetype.chunkLayout[i].offset = static_cast<uint32>(offset);
        archetype.chunkLayout[i].width = meta.size;

        offset += meta.size * archetype.maxEntitiesPerChunk;
    }

    UP_ASSERT(offset <= sizeof(Chunk::data));
}
