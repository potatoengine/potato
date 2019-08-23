// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/utility.h"
#include "potato/spud/sort.h"
#include <algorithm>

#include "archetype.h"
#include "entity.h"

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
}

template <typename Type, typename Projection = up::identity>
constexpr auto hashComponents(up::span<Type> input, Projection const& proj = {}, up::uint64 seed = 0) noexcept -> up::uint64 {
    for (Type const& value : input) {
        seed ^= static_cast<up::uint64>(up::project(proj, value));
    }
    return seed;
}

up::World::World() = default;

up::World::~World() = default;

void up::World::_populateArchetype(uint32 archetypeIndex, view<ComponentMeta const*> components) {
    Archetype& archetype = *_archetypes[archetypeIndex];

    archetype.layout.resize(components.size());

    // we'll always include the EntityId is the layout, so include its size
    size_t size = sizeof(EntityId);

    for (size_t i = 0; i != components.size(); ++i) {
        ComponentMeta const& meta = *components[i];

        archetype.layout[i].component = meta.id;
        archetype.layout[i].meta = &meta;
        size = align(size, meta.alignment);
        size += meta.size;
    }
    UP_ASSERT(size <= sizeof(Chunk::data));

    if (size != 0) {
        _calculateLayout(archetype, size);
    }

    // Required for partial match algorithm
    sort(archetype.layout, {}, &ChunkLayout::component);
}

void up::World::_calculateLayout(Archetype& archetype, size_t size) {
    // calculate layout hash
    archetype.layoutHash = hashComponents(span{archetype.layout}, &ChunkLayout::component);

    // assign pointer offers by alignment
    up::sort(archetype.layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

    // FIXME: figure out why we need this size + 1, the arithmetic surprises me
    archetype.perChunk = static_cast<uint32>(sizeof(ChunkPayload) / (size + 1));

    // we'll always include the EntityId is the layout, so include it as offset
    size_t offset = sizeof(EntityId) * archetype.perChunk;

    for (size_t i = 0; i != archetype.layout.size(); ++i) {
        ComponentMeta const& meta = *archetype.layout[i].meta;

        // align as required (requires alignment to be a power of 2)
        UP_ASSERT((meta.alignment & (meta.alignment - 1)) == 0);
        offset = align(offset, meta.alignment);
        UP_ASSERT(offset + archetype.perChunk * meta.size + meta.size <= sizeof(ChunkPayload));

        archetype.layout[i].offset = static_cast<uint32>(offset);
        archetype.layout[i].width = meta.size;

        offset += meta.size * archetype.perChunk;
    }

    UP_ASSERT(offset <= sizeof(Chunk::data));
}

void up::World::deleteEntity(EntityId entity) noexcept {
    Location location;
    if (!_tryGetLocation(entity, location)) {
        return;
    }

    _deleteLocation(location);
    _recycleEntityId(entity);
}

void up::World::_deleteLocation(Location const& location) noexcept {
    auto lastChunkIndex = location.archetype->chunks.size() - 1;
    Chunk& lastChunk = *location.archetype->chunks[lastChunkIndex];

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    if (location.entityIndex == location.archetype->count) {
        auto lastSubIndex = lastChunk.header.count - 1;

        EntityId lastEntity = *_stream<EntityId>(location.chunk->data, 0, location.subIndex) = *_stream<EntityId>(lastChunk.data, 0, lastSubIndex);
        _entityMapping[getEntityMappingIndex(lastEntity)].index = location.entityIndex;

        for (auto const& layout : location.archetype->layout) {
            ComponentMeta const& meta = *layout.meta;

            void* pointer = _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);
            void* lastPointer = _stream(lastChunk.data, layout.offset, layout.width, lastSubIndex);

            meta.relocate(pointer, lastPointer);
            meta.destroy(lastPointer);
        }
    }
    else {
        // We were already the last element, just just clean up our memory
        for (auto const& layout : location.archetype->layout) {
            ComponentMeta const& meta = *layout.meta;

            void* lastPointer = _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);

            meta.destroy(lastPointer);
        }
    }

    if (--lastChunk.header.count == 0) {
        _recycleChunk(std::move(location.archetype->chunks[lastChunkIndex]));
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
    auto iter = find(location.archetype->layout, componentId, {}, &ChunkLayout::component);
    if (iter == location.archetype->layout.end()) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[maxArchetypeComponents];
    copy(location.archetype->layout.begin(), iter, newComponentMetas, &ChunkLayout::meta);
    copy(iter + 1, location.archetype->layout.end(), newComponentMetas, &ChunkLayout::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->layout.size() - 1);

    // find the target archetype and allocate an entry in it
    auto newArchetypeIndex = _findArchetypeIndex(newComponentMetaSpan);
    Archetype& newArchetype = *_archetypes[newArchetypeIndex];

    auto newEntityIndex = newArchetype.count++;
    auto newChunkIndex = newEntityIndex / newArchetype.perChunk;
    auto newSubIndex = newEntityIndex % newArchetype.perChunk;

    if (newChunkIndex == newArchetype.chunks.size()) {
        newArchetype.chunks.push_back(_allocateChunk());
    }

    Chunk& newChunk = *newArchetype.chunks[newChunkIndex];
    ++newChunk.header.count;

    // copy components from old entity to new entity
    *_stream<EntityId>(newChunk.data, 0, newSubIndex) = entityId;

    for (ChunkLayout const& layout : newArchetype.layout) {
        int32 originalLayoutIndex = location.archetype->indexOfLayout(layout.component);
        UP_ASSERT(originalLayoutIndex != -1);

        auto const originalLayout = location.archetype->layout[originalLayoutIndex];

        void* newRawPointer = _stream(newChunk.data, layout.offset, layout.width, newSubIndex);
        void* originalRawPointer = _stream(location.chunk->data, originalLayout.offset, originalLayout.offset, location.subIndex);

        layout.meta->relocate(newRawPointer, originalRawPointer);
    }

    // remove old entity
    _deleteLocation(location);

    // update mapping
    uint32 entityMappingIndex = getEntityMappingIndex(entityId);
    _entityMapping[entityMappingIndex].archetype = newArchetypeIndex;
    _entityMapping[entityMappingIndex].index = newEntityIndex;
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept {
    Location location;
    if (!_tryGetLocation(entityId, location)) {
        return;
    }

    // construct the list of components that will exist in the new archetype
    ComponentMeta const* newComponentMetas[maxArchetypeComponents];
    newComponentMetas[0] = componentMeta;
    copy(location.archetype->layout.begin(), location.archetype->layout.end(), newComponentMetas + 1, &ChunkLayout::meta);
    span newComponentMetaSpan = span{newComponentMetas}.first(location.archetype->layout.size() + 1);

    // find the target archetype and allocate an entry in it
    auto newArchetypeIndex = _findArchetypeIndex(newComponentMetaSpan);
    Archetype& newArchetype = *_archetypes[newArchetypeIndex];

    auto newEntityIndex = newArchetype.count++;
    auto newChunkIndex = newEntityIndex / newArchetype.perChunk;
    auto newSubIndex = newEntityIndex % newArchetype.perChunk;

    if (newChunkIndex == newArchetype.chunks.size()) {
        newArchetype.chunks.push_back(_allocateChunk());
    }

    Chunk& newChunk = *newArchetype.chunks[newChunkIndex];
    ++newChunk.header.count;

    // copy components from old entity to new entity
    *_stream<EntityId>(newChunk.data, 0, newSubIndex) = entityId;

    for (ChunkLayout const& layout : newArchetype.layout) {
        void* newRawPointer = _stream(newChunk.data, layout.offset, layout.width, newSubIndex);

        // either copy existing component or install the new one
        int32 originalLayoutIndex = location.archetype->indexOfLayout(layout.component);
        if (originalLayoutIndex != -1) {
            auto const originalLayout = location.archetype->layout[originalLayoutIndex];

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
    uint32 entityMappingIndex = getEntityMappingIndex(entityId);
    _entityMapping[entityMappingIndex].archetype = newArchetypeIndex;
    _entityMapping[entityMappingIndex].index = newEntityIndex;
}

auto up::World::archetypes() const noexcept -> view<box<Archetype>> {
    return _archetypes;
}

auto up::World::_findArchetypeIndex(view<ComponentMeta const*> components) noexcept -> up::uint32 {
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

void up::World::forEachChunk(ArchetypeId archetypeId, delegate_ref<ForEachChunkSignature> callback) const {
    Archetype const& archetype = *_archetypes[static_cast<uint32>(archetypeId)];

    for (uint32 chunkIndex = 0; chunkIndex != archetype.chunks.size(); ++chunkIndex) {
        callback(archetype.chunks[chunkIndex].get());
    }
}

auto up::World::selectArchetypes(view<ComponentId> componentIds, delegate_ref<SelectSignature> callback) const -> int {
    int offsets[maxArchetypeComponents];

    for (int index = 0; index != _archetypes.size(); ++index) {
        auto const& archetype = *_archetypes[index];
        bool matched = true;

        for (int component = 0; component != componentIds.size(); ++component) {
            auto const layoutIndex = archetype.indexOfLayout(componentIds[component]);
            if (layoutIndex == -1) {
                matched = false;
                break;
            }
            offsets[component] = archetype.layout[layoutIndex].offset;
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

    UP_ASSERT(components.size() == archetype.layout.size());
    UP_ASSERT(components.size() == data.size());

    uint32 entityIndex = archetype.count++;

    // Determine destination location Chunk
    auto chunkIndex = entityIndex / archetype.perChunk;
    auto subIndex = entityIndex % archetype.perChunk;

    UP_ASSERT(chunkIndex <= archetype.chunks.size());

    if (chunkIndex == archetype.chunks.size()) {
        archetype.chunks.push_back(_allocateChunk());
    }

    Chunk& chunk = *archetype.chunks[chunkIndex];
    ++chunk.header.count;

    // Allocate EntityId
    EntityId entity = _allocateEntityId(archetypeIndex, entityIndex);

    // Copy component data and Entity data
    *_stream<EntityId>(chunk.data, 0, subIndex) = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentMeta const& meta = *components[index];

        int32 layoutIndex = archetype.indexOfLayout(meta.id);
        UP_ASSERT(archetype.layout[layoutIndex].meta == &meta);
        UP_ASSERT(archetype.layout[layoutIndex].width == meta.size);
        UP_ASSERT(layoutIndex != -1);

        auto const layout = archetype.layout[layoutIndex];

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

    int32 layoutIndex = location.archetype->indexOfLayout(component);
    if (layoutIndex == -1) {
        return nullptr;
    }

    auto const layout = location.archetype->layout[layoutIndex];

    return _stream(location.chunk->data, layout.offset, layout.width, location.subIndex);
}

void up::World::_selectChunksRaw(up::uint32 archetypeIndex, up::uint32 chunkIndex, view<ComponentId> components, size_t& out_count, span<void*> outputPointers) const {
    UP_ASSERT(components.size() <= maxSelectComponents);

    Archetype const& archetype = *_archetypes[archetypeIndex];

    int32 layoutIndices[maxSelectComponents];
    for (size_t i = 0; i < components.size(); ++i) {
        layoutIndices[i] = archetype.indexOfLayout(components[i]);
        UP_ASSERT(layoutIndices[i] != -1);
    }

    box<Chunk> const& chunk = _archetypes[archetypeIndex]->chunks[chunkIndex];

    EntityId const* entities = _stream<EntityId>(chunk->data, 0, 0);

    for (size_t i = 0; i < components.size(); ++i) {
        auto const layout = archetype.layout[layoutIndices[i]];
        outputPointers[i] = _stream(chunk->data, layout.offset);
    }

    out_count = static_cast<size_t>(chunk->header.count);
    //callback(static_cast<size_t>(chunk->header.count), entities, view<void*>(pointers).first(components.size()));
}

auto up::World::_allocateEntityId(uint32 archetypeIndex, uint32 entityIndex) noexcept -> EntityId {
    // if there's a free ID, recycle it
    if (_freeEntityHead != freeEntityIndex) {
        uint32 mappingIndex = _freeEntityHead;
        _freeEntityHead = _entityMapping[mappingIndex].index;

        _entityMapping[mappingIndex].archetype = archetypeIndex;
        _entityMapping[mappingIndex].index = entityIndex;

        return makeEntityId(mappingIndex, _entityMapping[mappingIndex].generation);
    }

    // there was no ID to recycle, so create a new one
    uint32 mappingIndex = static_cast<uint32>(_entityMapping.size());

    Entity entity;
    entity.generation = 1;
    entity.archetype = archetypeIndex;
    entity.index = entityIndex;
    _entityMapping.push_back(entity);

    return makeEntityId(mappingIndex, entity.generation);
}

void up::World::_recycleEntityId(EntityId entity) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);

    ++_entityMapping[entityMappingIndex].generation;
    _entityMapping[entityMappingIndex].index = _freeEntityHead;

    _freeEntityHead = entityMappingIndex;
}

auto up::World::_allocateChunk() -> box<Chunk> {
    if (!_freeChunks.empty()) {
        auto chunk = std::move(_freeChunks.back());
        _freeChunks.pop_back();
        return chunk;
    }

    return new_box<Chunk>();
}

void up::World::_recycleChunk(box<Chunk> chunk) {
    _freeChunks.push_back(std::move(chunk));
}

auto up::World::_tryGetLocation(EntityId entityId, Location& location) const noexcept -> bool {
    uint32 entityMappingIndex = getEntityMappingIndex(entityId);
    if (entityMappingIndex >= _entityMapping.size()) {
        return false;
    }

    Entity const& entity = _entityMapping[entityMappingIndex];

    if (entity.generation != getEntityGeneration(entityId)) {
        return false;
    }

    location.archetypeIndex = entity.archetype;
    UP_ASSERT(location.archetypeIndex < _archetypes.size());
    location.archetype = _archetypes[location.archetypeIndex].get();

    uint32 perChunk = location.archetype->perChunk;
    location.chunkIndex = entity.index / perChunk;
    location.subIndex = entity.index % perChunk;

    UP_ASSERT(location.chunkIndex < location.archetype->chunks.size());
    location.chunk = location.archetype->chunks[location.chunkIndex].get();
    UP_ASSERT(location.subIndex < location.chunk->header.count);

    return true;
}
