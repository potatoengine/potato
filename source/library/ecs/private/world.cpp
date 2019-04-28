// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/foundation/find.h"
#include "potato/foundation/hash.h"
#include "potato/foundation/utility.h"
#include "potato/foundation/sort.h"
#include <algorithm>

#include "archetype.h"
#include "chunk.h"
#include "entity.h"

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
}

template <typename Type, typename Projection = up::identity>
constexpr auto hashComponents(up::span<Type> input, Projection const& proj = {}) noexcept -> up::uint64 {
    up::uint64 hash = 0;
    for (Type const& value : input) {
        hash ^= static_cast<up::uint64>(up::project(proj, value));
    }
    return hash;
}

up::World::World() = default;

up::World::~World() {
    while (_freeChunkHead != nullptr) {
        Chunk* chunk = _freeChunkHead;
        _freeChunkHead = _freeChunkHead->header.next;

        delete chunk;
    }
}

void up::World::_calculateLayout(uint32 archetypeIndex, view<ComponentMeta const*> components) {
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

    // calculate layout hash
    archetype.layoutHash = hashComponents(span{archetype.layout}, &Layout::component);

    if (size != 0) {
        // assign pointer offers by alignment
        up::sort(archetype.layout, {}, [](const auto& layout) noexcept { return layout.meta->alignment; });

        // FIXME: figure out why we need this size + 1, the arithmetic surprises me
        archetype.perChunk = static_cast<uint32>(sizeof(Chunk::Payload) / (size + 1));

        // we'll always include the EntityId is the layout, so include it as offset
        size_t offset = sizeof(EntityId) * archetype.perChunk;

        for (size_t i = 0; i != components.size(); ++i) {
            ComponentMeta const& meta = *components[i];

            // align as required (requires alignment to be a power of 2)
            UP_ASSERT((meta.alignment & (meta.alignment - 1)) == 0);
            offset = align(offset, meta.alignment);
            UP_ASSERT(offset + archetype.perChunk * meta.size + meta.size <= sizeof(Chunk::Payload));

            archetype.layout[i].offset = static_cast<uint32>(offset);
            archetype.layout[i].width = meta.size;

            offset += meta.size * archetype.perChunk;
        }

        UP_ASSERT(offset <= sizeof(Chunk::data));
    }

    // Required for partial match algorithm
    sort(archetype.layout, {}, &Layout::component);
}

void up::World::deleteEntity(EntityId entity) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);

    UP_ASSERT(entityMappingIndex < _entityMapping.size());

    Entity const& mapping = _entityMapping[entityMappingIndex];
    uint32 archetypeIndex = mapping.archetype;
    uint32 entityIndex = mapping.index;

    UP_ASSERT(mapping.generation == getEntityGeneration(entity));

    Archetype& archetype = *_archetypes[archetypeIndex];

    auto chunkIndex = entityIndex / archetype.perChunk;
    auto subIndex = entityIndex % archetype.perChunk;

    UP_ASSERT(chunkIndex < archetype.chunks.size());

    Chunk& chunk = *archetype.chunks[chunkIndex];

    auto lastChunkIndex = archetype.chunks.size() - 1;
    Chunk& lastChunk = *archetype.chunks[lastChunkIndex];

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    if (entityIndex == archetype.count - 1) {
        auto lastSubIndex = lastChunk.header.count - 1;

        EntityId lastEntity = *chunk.entity(subIndex) = *lastChunk.entity(lastSubIndex);
        _entityMapping[getEntityMappingIndex(lastEntity)].index = entityIndex;

        for (auto const& layout : archetype.layout) {
            ComponentMeta const& meta = *layout.meta;

            void* pointer = chunk.pointer(layout, subIndex);
            void* lastPointer = lastChunk.pointer(layout, lastSubIndex);
            meta.relocate(pointer, lastPointer);
            meta.destroy(lastPointer);
        }
    }
    else {
        // We were already the last element, just just clean up our memory
        for (auto const& layout : archetype.layout) {
            ComponentMeta const& meta = *layout.meta;

            void* lastPointer = chunk.pointer(layout, subIndex);
            meta.destroy(lastPointer);
        }
    }

    if (--lastChunk.header.count == 0) {
        _recycleChunk(std::move(archetype.chunks[lastChunkIndex]));
        archetype.chunks.pop_back();
    }
}

auto up::World::archetypes() const noexcept -> view<box<Archetype>> {
    return _archetypes;
}

bool up::World::_matchArchetype(uint32 archetypeIndex, view<ComponentId> sortedComponents) const noexcept {
    Archetype const& archetype = *_archetypes[archetypeIndex];

    uint32 layoutIndex = 0;
    uint32 layoutCount = static_cast<uint32>(archetype.layout.size());

    UP_ASSERT(layoutCount != 0);

    // for each component, find an appropriate entry in the layout (which can only be found once).
    // algorithm relies on layout components and 
    for (ComponentId id : sortedComponents) {
        if (layoutIndex == layoutCount) {
            return false;
        }

        // seek forward skipping components in layout not required by request
        while (archetype.layout[layoutIndex].meta->id < id) {
            if (++layoutIndex == layoutCount) {
                return false;
            }
        }

        // if the next component is not a match, the layout is missing a required component
        if (archetype.layout[layoutIndex].meta->id > id) {
            return false;
        }

        ++layoutIndex;
    }

    return true;
}

auto up::World::_findArchetypeIndex(view<ComponentMeta const*> components) noexcept -> up::uint32 {
    uint64 hash = hashComponents(components, &ComponentMeta::id);

    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_archetypes[index]->layoutHash == hash) {
            return index;
        }
    }

    auto arch = new_box<Archetype>();
    _archetypes.push_back(std::move(arch));
    uint32 archetypeIndex = static_cast<uint32>(_archetypes.size() - 1);
    _calculateLayout(archetypeIndex, components);

    UP_ASSERT(_archetypes[archetypeIndex]->layoutHash == hash);

    return archetypeIndex;
}

void up::World::_selectRaw(view<ComponentId> sortedComponents, delegate_ref<RawSelectSignature> callback) const {
    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_matchArchetype(index, sortedComponents)) {
            _selectChunksRaw(index, sortedComponents, callback);
        }
    }
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
    *chunk.entity(subIndex) = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentMeta const& meta = *components[index];

        int32 layoutIndex = archetype.indexOfLayout(meta.id);
        UP_ASSERT(layoutIndex != -1);

        void* rawPointer = chunk.pointer(archetype.layout[layoutIndex], subIndex);

        meta.copy(rawPointer, data[index]);
    }

    return entity;
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);
    if (entityMappingIndex >= _entityMapping.size()) {
        return nullptr;
    }

    Entity const& mapping = _entityMapping[entityMappingIndex];
    if (mapping.generation != getEntityGeneration(entity)) {
        return nullptr;
    }

    // FIXME: bounds check
    return _getComponentPointer(mapping.archetype, mapping.index, component);
}

void up::World::_selectChunksRaw(up::uint32 archetypeIndex, view<ComponentId> components, delegate_ref<RawSelectSignature> callback) const {
    UP_ASSERT(components.size() <= maxSelectComponents);

    Archetype const& archetype = *_archetypes[archetypeIndex];

    int32 layoutIndices[maxSelectComponents];
    for (size_t i = 0; i < components.size(); ++i) {
        layoutIndices[i] = archetype.indexOfLayout(components[i]);
        UP_ASSERT(layoutIndices[i] != -1);
    }

    void* pointers[maxSelectComponents];
    for (box<Chunk> const& chunk : _archetypes[archetypeIndex]->chunks) {
        EntityId const* entities = chunk->entity(0);

        for (size_t i = 0; i < components.size(); ++i) {
            pointers[i] = chunk->pointer(archetype.layout[layoutIndices[i]], 0);
        }

        callback(static_cast<size_t>(chunk->header.count), entities, view<void*>(pointers).first(components.size()));
    }
}

void* up::World::_getComponentPointer(up::uint32 archetypeIndex, up::uint32 entityIndex, ComponentId component) const noexcept {
    Archetype& archetype = *_archetypes[archetypeIndex];

    int32 layoutIndex = archetype.indexOfLayout(component);
    UP_ASSERT(layoutIndex != -1);

    auto chunkIndex = entityIndex / archetype.perChunk;
    auto subIndex = entityIndex % archetype.perChunk;

    return archetype.chunks[chunkIndex]->pointer(archetype.layout[layoutIndex], entityIndex);
}

auto up::World::_allocateEntityId(uint32 archetypeIndex, uint32 entityIndex) noexcept -> EntityId {
    if (_freeEntityHead == static_cast<decltype(_freeEntityHead)>(-1)) {
        // nothing in free list
        Entity entity;
        entity.generation = 1;
        entity.archetype = archetypeIndex;
        entity.index = entityIndex;

        uint32 mappingIndex = static_cast<uint32>(_entityMapping.size());
        _entityMapping.push_back(entity);

        return makeEntityId(mappingIndex, entity.generation);
    }

    uint32 mappingIndex = _freeEntityHead;
    _freeEntityHead = _entityMapping[mappingIndex].index;

    _entityMapping[mappingIndex].archetype = archetypeIndex;
    _entityMapping[mappingIndex].index = entityIndex;

    return makeEntityId(mappingIndex, _entityMapping[mappingIndex].generation);
}

void up::World::_recycleEntityId(EntityId entity) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);

    ++_entityMapping[entityMappingIndex].generation;
    _entityMapping[entityMappingIndex].index = _freeEntityHead;

    _freeEntityHead = entityMappingIndex;
}

auto up::World::_allocateChunk() -> box<Chunk> {
    static_assert(sizeof(up::World::Chunk) == up::World::Chunk::size);

    if (_freeChunkHead != nullptr) {
        box<Chunk> chunk(_freeChunkHead);
        _freeChunkHead = _freeChunkHead->header.next;
        return chunk;
    }

    return new_box<Chunk>();
}

void up::World::_recycleChunk(box<Chunk> chunk) {
    chunk->header.next = _freeChunkHead;
    _freeChunkHead = chunk.release();
}
