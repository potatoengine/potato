// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include "potato/spud/find.h"
#include "potato/runtime/assertion.h"

auto up::ArchetypeMapper::_matchArchetype(Archetype const& arch, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool {
    UP_ASSERT(componentIds.size() == offsets.size());

    auto const layout = _layout.subspan(arch.layoutOffset, arch.layoutLength);

    for (ComponentId const component : componentIds) {
        auto const desc = find(layout, component, {}, &ChunkRowDesc::component);
        if (desc == layout.end()) {
            return false;
        }
        offsets.front() = desc->offset;
        offsets.pop_front();
    }
    return true;
};

void up::ArchetypeMapper::_calculateLayout(Archetype& archetype, view<ComponentMeta const*> components) {
    archetype.layoutOffset = static_cast<uint32>(_layout.size());
    archetype.layoutLength = static_cast<uint16>(components.size() + 1);

    _layout.reserve(archetype.layoutOffset + archetype.layoutLength);

    // we'll always include the EntityId in the layout
    //
    auto const entityMeta = ComponentMeta::get<Entity>();
    _layout.push_back({entityMeta->id, entityMeta, 0, static_cast<uint16>(entityMeta->size)});
    _layout.back().meta = entityMeta;

    // append all the other components
    //
    for (ComponentMeta const* meta : components) {
        _layout.push_back({meta->id, meta, 0, static_cast<uint16>(meta->size)});
    }

    auto const layout = _layout.subspan(archetype.layoutOffset, archetype.layoutLength);

    // sort rows by alignment for ideal packing
    //
    up::sort(layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

    // calculate total size of all components
    //
    size_t size = 0;
    size_t padding = 0;
    for (auto const& row : layout) {
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
    for (auto& row : layout) {
        offset = align_to(offset, row.meta->alignment);
        row.offset = static_cast<uint32>(offset);
        offset += row.width * archetype.maxEntitiesPerChunk;
        UP_ASSERT(offset <= sizeof(Chunk::data));
    }
}

auto up::ArchetypeMapper::createArchetype(view<ComponentMeta const*> components) -> Archetype* {
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
    _calculateLayout(arch, components);

    _archetypes.push_back(std::move(arch));

    return &_archetypes.back();
}

auto up::ArchetypeMapper::selectArchetypes(view<ComponentId> componentIds, span<int> offsetsBuffer, size_t start, delegate_ref<SelectSignature> callback) const noexcept -> size_t {
    size_t index = start;
    while (index < _archetypes.size()) {
        auto& arch = _archetypes[index++];
        if (_matchArchetype(arch, componentIds, offsetsBuffer)) {
            callback(arch.id, offsetsBuffer.first(componentIds.size()));
        }
    }

    return index;
}

auto up::ArchetypeMapper::addChunk(Archetype& arch, Chunk* chunk) -> size_t {
    _chunks.insert(_chunks.begin() + arch.chunksOffset + arch.chunksLength, chunk);
    ++arch.chunksLength;

    for (size_t index = static_cast<size_t>(arch.id); index != _archetypes.size(); ++index) {
        ++_archetypes[index].chunksOffset;
    }

    return arch.chunksLength - 1;
}

void up::ArchetypeMapper::removeChunk(Archetype& arch, size_t chunkIndex) noexcept {
    _chunks.erase(_chunks.begin() + arch.chunksOffset + chunkIndex);
    --arch.chunksLength;

    for (size_t index = static_cast<size_t>(arch.id); index != _archetypes.size(); ++index) {
        --_archetypes[index].chunksOffset;
    }
}
