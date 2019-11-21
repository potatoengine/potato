// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include "potato/spud/find.h"
#include "potato/runtime/assertion.h"

up::ArchetypeMapper::ArchetypeMapper() {
    // Archetype 0 is the empty archetype
    _archetypes.emplace_back();
}

auto up::ArchetypeMapper::_matchArchetype(ArchetypeId archetype, view<ComponentId> componentIds, span<int> offsets) const noexcept -> bool {
    UP_ASSERT(componentIds.size() == offsets.size());

    auto const layout = layoutOf(archetype);

    for (ComponentId const component : componentIds) {
        auto const desc = find(layout, component, {}, &ChunkRowDesc::component);
        if (desc == layout.end()) {
            return false;
        }
        offsets.front() = desc->offset;
        offsets.pop_front();
    }
    return true;
}

auto up::ArchetypeMapper::_beginArchetype() -> ArchetypeId {
    // bump so Query objects know that the list of archetypes has changed
    //
    ++_version;

    // allocate a new archetype and associated id
    //
    auto id = ArchetypeId(_archetypes.size());
    auto& archData = *_archetypes.emplace_back();

    archData.chunksOffset = static_cast<uint32>(_chunks.size());
    archData.layoutOffset = static_cast<uint32>(_layout.size());

    // we'll always include the EntityId in the layout
    //
    auto const entityMeta = ComponentMeta::get<Entity>();
    _layout.push_back({entityMeta->id, entityMeta, 0, static_cast<uint16>(entityMeta->size)});
    _layout.back().meta = entityMeta;

    return id;
}

auto up::ArchetypeMapper::_finalizeArchetype(ArchetypeId archetype) noexcept -> ArchetypeId {
    auto& archData = _archetypes[to_underlying(archetype)];
    archData.layoutLength = static_cast<uint16>(_layout.size() - archData.layoutOffset);
    auto const layout = _layout.subspan(archData.layoutOffset, archData.layoutLength);

    // sort rows by alignment for ideal packing
    //
    up::sort(
        layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

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
    archData.maxEntitiesPerChunk = static_cast<uint32>((sizeof(ChunkPayload) - padding) / size);
    UP_ASSERT(archData.maxEntitiesPerChunk > 0);

    // calculate the row offets for the layout
    //
    size_t offset = 0;
    for (auto& row : layout) {
        offset = align_to(offset, row.meta->alignment);
        row.offset = static_cast<uint32>(offset);
        offset += row.width * archData.maxEntitiesPerChunk;
        UP_ASSERT(offset <= sizeof(Chunk::data));
    }

    return archetype;
}

template <typename P>
auto up::ArchetypeMapper::_findArchetype(P&& predicate) noexcept -> Archetype* {
    for (Archetype& arch : _archetypes) {
        auto const layout = _layout.subspan(arch.layoutOffset, arch.layoutLength);
        if (predicate(layout, arch)) {
            return &arch;
        }
    }
    return nullptr;
}

auto up::ArchetypeMapper::acquireArchetype(view<ComponentMeta const*> components) -> ArchetypeId {
    auto arch = _findArchetype([components](view<ChunkRowDesc> layout, Archetype const& arch) noexcept {
        if (layout.size() - 1 /*Entity*/ != components.size()) {
            return false;
        }
        for (ComponentMeta const* meta : components) {
            if (!contains(layout, meta->id, {}, &ChunkRowDesc::component)) {
                return false;
            }
        }
        return true;
    });
    if (arch != nullptr) {
        return static_cast<ArchetypeId>(arch - _archetypes.begin());
    }

    // bump so Query objects know that the list of archetypes has changed
    //
    ++_version;

    auto id = _beginArchetype();

    // append all the other components
    //
    for (ComponentMeta const* meta : components) {
        _layout.push_back({meta->id, meta, 0, static_cast<uint16>(meta->size)});
    }

    return _finalizeArchetype(id);
}

auto up::ArchetypeMapper::acquireArchetypeWith(ArchetypeId original, ComponentMeta const* additional) -> ArchetypeId {
    auto arch = _findArchetype([originalLayout = layoutOf(original), additional](view<ChunkRowDesc> layout, Archetype const& arch) noexcept {
        if (layout.size() - 1 /*additional*/ != originalLayout.size() - 1 /*Entity*/) {
            return false;
        }
        if (!contains(layout, additional->id, {}, &ChunkRowDesc::component)) {
            return false;
        }
        for (ChunkRowDesc const& row : originalLayout) {
            if (!contains(layout, row.component, {}, &ChunkRowDesc::component)) {
                return false;
            }
        }
        return true;
    });
    if (arch != nullptr) {
        return static_cast<ArchetypeId>(arch - _archetypes.begin());
    }

    // bump so Query objects know that the list of archetypes has changed
    //
    ++_version;

    auto id = _beginArchetype();
    _layout.pop_back(); // entity id
    auto const& originalArch = _archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        _layout.push_back(_layout[originalArch.layoutOffset + index]);
    }
    _layout.push_back({additional->id, additional, 0, static_cast<uint16>(additional->size)});

    return _finalizeArchetype(id);
}

auto up::ArchetypeMapper::acquireArchetypeWithout(ArchetypeId original, ComponentId excluded) -> ArchetypeId {
    auto arch = _findArchetype([originalLayout = layoutOf(original), excluded](view<ChunkRowDesc> layout, Archetype const& arch) noexcept {
        if (layout.size() != originalLayout.size() - 2 /*Entity, excluded*/) {
            return false;
        }
        for (ChunkRowDesc const& row : originalLayout) {
            if (row.component == excluded || !contains(layout, row.component, {}, &ChunkRowDesc::component)) {
                return false;
            }
        }
        return true;
    });
    if (arch != nullptr) {
        return static_cast<ArchetypeId>(arch - _archetypes.begin());
    }

    auto id = _beginArchetype();
    _layout.pop_back(); // entity id
    auto const& originalArch = _archetypes[to_underlying(original)];
    for (int index = 0; index != originalArch.layoutLength; ++index) {
        auto const& row = _layout[originalArch.layoutOffset + index];
        if (row.component != excluded) {
            _layout.push_back(row);
        }
    }
    return _finalizeArchetype(id);
}

auto up::ArchetypeMapper::addChunk(ArchetypeId arch, Chunk* chunk) -> int {
    UP_ASSERT(arch == chunk->header.archetype);

    Archetype& archData = _archetypes[to_underlying(arch)];
    int const lastIndex = archData.chunksLength;

    chunk->header.capacity = archData.maxEntitiesPerChunk;

    _chunks.insert(_chunks.begin() + archData.chunksOffset + archData.chunksLength, chunk);
    ++archData.chunksLength;

    for (auto index = static_cast<size_t>(arch) + 1; index != _archetypes.size(); ++index) {
        ++_archetypes[index].chunksOffset;
    }

    return lastIndex;
}

void up::ArchetypeMapper::removeChunk(ArchetypeId arch, int chunkIndex) noexcept {
    Archetype& archData = _archetypes[to_underlying(arch)];

    _chunks.erase(_chunks.begin() + archData.chunksOffset + chunkIndex);
    --archData.chunksLength;

    for (auto index = static_cast<size_t>(arch) + 1; index != _archetypes.size(); ++index) {
        --_archetypes[index].chunksOffset;
    }
}
