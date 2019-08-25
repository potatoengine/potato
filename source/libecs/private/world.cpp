// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include <algorithm>

#include "location.h"

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
}

template <typename Type, typename Projection = up::identity>
static constexpr auto hashComponents(up::span<Type> input, Projection const& proj = {}, up::uint64 seed = 0) noexcept -> up::uint64 {
    for (Type const& value : input) {
        seed ^= static_cast<up::uint64>(up::project(proj, value));
    }
    return seed;
}

up::World::World() = default;

up::World::~World() = default;

auto up::World::getArchetype(ArchetypeId arch) const noexcept -> Archetype const* {
    auto const index = to_underlying(arch);

    if (index < 0 || index >= _archetypes.size()) {
        return nullptr;
    }

    return _archetypes[index].get();
}

void up::World::_populateArchetype(uint32 archetypeIndex, view<ComponentMeta const*> components) {
    Archetype& archetype = *_archetypes[archetypeIndex];

    archetype.chunkLayout.resize(components.size());

    // we'll always include the EntityId is the layout, so include its size
    size_t size = sizeof(EntityId);

    for (size_t i = 0; i != components.size(); ++i) {
        ComponentMeta const& meta = *components[i];

        archetype.chunkLayout[i].component = meta.id;
        archetype.chunkLayout[i].meta = &meta;
        size = align(size, meta.alignment);
        size += meta.size;
    }
    UP_ASSERT(size <= sizeof(Chunk::data));

    if (size != 0) {
        _calculateLayout(archetype, size);
    }

    // Required for partial match algorithm
    sort(archetype.chunkLayout, {}, &ChunkLayout::component);
}

void up::World::_calculateLayout(Archetype& archetype, size_t size) {
    // calculate layout hash
    archetype.layoutHash = hashComponents(span{archetype.chunkLayout}, &ChunkLayout::component);

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
        offset = align(offset, meta.alignment);
        UP_ASSERT(offset + archetype.maxEntitiesPerChunk * meta.size + meta.size <= sizeof(ChunkPayload));

        archetype.chunkLayout[i].offset = static_cast<uint32>(offset);
        archetype.chunkLayout[i].width = meta.size;

        offset += meta.size * archetype.maxEntitiesPerChunk;
    }

    UP_ASSERT(offset <= sizeof(Chunk::data));
}

void up::World::deleteEntity(EntityId entity) noexcept {
    Location location;
    if (!_tryGetLocation(entity, location)) {
        return;
    }

    _deleteLocation(location);
    _entities.recycle(entity);
}

void up::World::_deleteLocation(Location const& location) noexcept {
    auto lastChunkIndex = location.archetype->chunks.size() - 1;
    Chunk& lastChunk = *location.archetype->chunks[lastChunkIndex];

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    if (location.entityIndex == location.archetype->entityCount) {
        auto lastSubIndex = lastChunk.header.entities - 1;

        EntityId lastEntity = *_stream<EntityId>(location.chunk->data, 0, location.subIndex) = *_stream<EntityId>(lastChunk.data, 0, lastSubIndex);
        _entities.setIndex(lastEntity, location.entityIndex);

        for (auto const& layout : location.archetype->chunkLayout) {
            ComponentMeta const& meta = *layout.meta;

            void* pointer = _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);
            void* lastPointer = _stream(lastChunk.data, layout.offset, layout.width, lastSubIndex);

            meta.relocate(pointer, lastPointer);
            meta.destroy(lastPointer);
        }
    }
    else {
        // We were already the last element, just just clean up our memory
        for (auto const& layout : location.archetype->chunkLayout) {
            ComponentMeta const& meta = *layout.meta;

            void* lastPointer = _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);

            meta.destroy(lastPointer);
        }
    }

    if (--lastChunk.header.entities == 0) {
        _chunks.recycle(std::move(location.archetype->chunks[lastChunkIndex]));
        location.archetype->chunks.pop_back();
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    Location location;
    if (!_tryGetLocation(entityId, location)) {
        return;
    }

    // figure out which layout entry is being removed
    // FIXME: if we have multiples of a component, this assumes the first one only
    auto iter = find(location.archetype->chunkLayout, componentId, {}, &ChunkLayout::component);
    if (iter == location.archetype->chunkLayout.end()) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[maxArchetypeComponents];
    copy(location.archetype->chunkLayout.begin(), iter, newComponentMetas, &ChunkLayout::meta);
    copy(iter + 1, location.archetype->chunkLayout.end(), newComponentMetas, &ChunkLayout::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->chunkLayout.size() - 1);

    // find the target archetype and allocate an entry in it
    auto newArchetypeIndex = _findArchetypeIndex(newComponentMetaSpan);
    Archetype& newArchetype = *_archetypes[newArchetypeIndex];

    auto newEntityIndex = newArchetype.entityCount++;
    auto newChunkIndex = newEntityIndex / newArchetype.maxEntitiesPerChunk;
    auto newSubIndex = newEntityIndex % newArchetype.maxEntitiesPerChunk;

    if (newChunkIndex == newArchetype.chunks.size()) {
        newArchetype.chunks.push_back(_chunks.allocate());
    }

    Chunk& newChunk = *newArchetype.chunks[newChunkIndex];
    ++newChunk.header.entities;

    // copy components from old entity to new entity
    *_stream<EntityId>(newChunk.data, 0, newSubIndex) = entityId;

    for (ChunkLayout const& layout : newArchetype.chunkLayout) {
        int32 originalLayoutIndex = _indexOfLayout(*location.archetype, layout.component);
        UP_ASSERT(originalLayoutIndex != -1);

        auto const originalLayout = location.archetype->chunkLayout[originalLayoutIndex];

        void* newRawPointer = _stream(newChunk.data, layout.offset, layout.width, newSubIndex);
        void* originalRawPointer = _stream(location.chunk->data, originalLayout.offset, originalLayout.offset, location.subIndex);

        layout.meta->relocate(newRawPointer, originalRawPointer);
    }

    // remove old entity
    _deleteLocation(location);

    // update mapping
    _entities.setArchetype(entityId, ArchetypeId(newArchetypeIndex), newEntityIndex);
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept {
    Location location;
    if (!_tryGetLocation(entityId, location)) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[maxArchetypeComponents];
    newComponentMetas[0] = componentMeta;
    copy(location.archetype->chunkLayout.begin(), location.archetype->chunkLayout.end(), newComponentMetas + 1, &ChunkLayout::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->chunkLayout.size() + 1);

    // find the target archetype and allocate an entry in it
    auto newArchetypeIndex = _findArchetypeIndex(newComponentMetaSpan);
    Archetype& newArchetype = *_archetypes[newArchetypeIndex];

    auto newEntityIndex = newArchetype.entityCount++;
    auto newChunkIndex = newEntityIndex / newArchetype.maxEntitiesPerChunk;
    auto newSubIndex = newEntityIndex % newArchetype.maxEntitiesPerChunk;

    if (newChunkIndex == newArchetype.chunks.size()) {
        newArchetype.chunks.push_back(_chunks.allocate());
    }

    Chunk& newChunk = *newArchetype.chunks[newChunkIndex];
    ++newChunk.header.entities;

    // copy components from old entity to new entity
    *_stream<EntityId>(newChunk.data, 0, newSubIndex) = entityId;

    for (ChunkLayout const& layout : newArchetype.chunkLayout) {
        void* newRawPointer = _stream(newChunk.data, layout.offset, layout.width, newSubIndex);

        // either copy existing component or install the new one
        int32 originalLayoutIndex = _indexOfLayout(*location.archetype, layout.component);
        if (originalLayoutIndex != -1) {
            auto const originalLayout = location.archetype->chunkLayout[originalLayoutIndex];

            void* originalRawPointer = _stream(location.chunk->data, originalLayout.offset, originalLayout.width, location.subIndex);

            layout.meta->relocate(newRawPointer, originalRawPointer);
        }
        else {
            layout.meta->copy(newRawPointer, componentData);
        }
    }

    // remove old entity
    _deleteLocation(location);

    // update mapping
    _entities.setArchetype(entityId, ArchetypeId(newArchetypeIndex), newEntityIndex);
}

auto up::World::archetypes() const noexcept -> view<box<Archetype>> {
    return _archetypes;
}

auto up::World::_findArchetypeIndex(view<ComponentMeta const*> components) noexcept -> up::int32 {
    uint64 hash = hashComponents(components, &ComponentMeta::id);

    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_archetypes[index]->layoutHash == hash) {
            return index;
        }
    }

    // ensure queries are re-run
    //
    ++_version;

    auto arch = new_box<Archetype>();
    _archetypes.push_back(std::move(arch));
    uint32 archetypeIndex = static_cast<uint32>(_archetypes.size() - 1);
    _populateArchetype(archetypeIndex, components);

    UP_ASSERT(_archetypes[archetypeIndex]->layoutHash == hash);

    return archetypeIndex;
}

auto up::World::_indexOfLayout(Archetype const& archetype, ComponentId component) noexcept -> up::int32 {
    for (int32 i = 0; i != static_cast<int32>(archetype.chunkLayout.size()); ++i) {
        if (archetype.chunkLayout[i].component == component) {
            return i;
        }
    }
    return -1;
}

auto up::World::selectArchetypes(view<ComponentId> componentIds, delegate_ref<SelectSignature> callback) const -> int {
    int offsets[maxArchetypeComponents];

    for (int index = 0; index != _archetypes.size(); ++index) {
        auto const& archetype = *_archetypes[index];
        bool matched = true;

        for (int component = 0; component != componentIds.size(); ++component) {
            auto const layoutIndex = _indexOfLayout(archetype, componentIds[component]);
            if (layoutIndex == -1) {
                matched = false;
                break;
            }
            offsets[component] = archetype.chunkLayout[layoutIndex].offset;
        }

        if (matched) {
            callback(ArchetypeId(index), span{offsets}.first(componentIds.size()));
        }
    }

    return 0;
}

auto up::World::_createEntityRaw(view<ComponentMeta const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    uint32 archetypeIndex = _findArchetypeIndex(components);

    Archetype& archetype = *_archetypes[archetypeIndex];

    UP_ASSERT(components.size() == archetype.chunkLayout.size());
    UP_ASSERT(components.size() == data.size());

    uint32 entityIndex = archetype.entityCount++;

    // Determine destination location Chunk
    auto chunkIndex = entityIndex / archetype.maxEntitiesPerChunk;
    auto subIndex = entityIndex % archetype.maxEntitiesPerChunk;

    UP_ASSERT(chunkIndex <= archetype.chunks.size());

    if (chunkIndex == archetype.chunks.size()) {
        archetype.chunks.push_back(_chunks.allocate());
    }

    Chunk& chunk = *archetype.chunks[chunkIndex];
    ++chunk.header.entities;

    // Allocate EntityId
    EntityId entity = _entities.allocate(ArchetypeId(archetypeIndex), entityIndex);

    // Copy component data and Entity data
    *_stream<EntityId>(chunk.data, 0, subIndex) = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentMeta const& meta = *components[index];

        int32 layoutIndex = _indexOfLayout(archetype, meta.id);
        UP_ASSERT(archetype.chunkLayout[layoutIndex].meta == &meta);
        UP_ASSERT(archetype.chunkLayout[layoutIndex].width == meta.size);
        UP_ASSERT(layoutIndex != -1);

        auto const layout = archetype.chunkLayout[layoutIndex];

        void* rawPointer = _stream(chunk.data, layout.offset, layout.width, subIndex);

        meta.copy(rawPointer, data[index]);
    }

    return entity;
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    Location location;
    if (!_tryGetLocation(entity, location)) {
        return nullptr;
    }

    int32 layoutIndex = _indexOfLayout(*location.archetype, component);
    if (layoutIndex == -1) {
        return nullptr;
    }

    auto const layout = location.archetype->chunkLayout[layoutIndex];

    return _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);
}

auto up::World::_tryGetLocation(EntityId entityId, Location& location) const noexcept -> bool {
    ArchetypeId archetype;
    uint32 index;

    if (!_entities.tryParse(entityId, archetype, index)) {
        return false;
    }

    location.archetypeIndex = to_underlying(archetype);
    UP_ASSERT(location.archetypeIndex < _archetypes.size());
    location.archetype = _archetypes[location.archetypeIndex].get();

    uint32 perChunk = location.archetype->maxEntitiesPerChunk;
    location.chunkIndex = index / perChunk;
    location.subIndex = index % perChunk;

    UP_ASSERT(location.chunkIndex < location.archetype->chunks.size());
    location.chunk = location.archetype->chunks[location.chunkIndex];
    UP_ASSERT(location.subIndex < location.chunk->header.entities);

    return true;
}
