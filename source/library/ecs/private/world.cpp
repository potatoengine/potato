// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/foundation/find.h"
#include <algorithm>

#include "archetype.h"
#include "chunk.h"
#include "entity.h"

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
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

        archetype.layout[i].meta = &meta;
        size = align(size, meta.alignment);
        size += meta.size;
    }
    UP_ASSERT(size <= sizeof(Chunk::data));

    if (size != 0) {
        // assign pointer offers by alignment
        std::sort(archetype.layout.begin(), archetype.layout.end(), [](const auto& l, const auto& r) noexcept { return l.meta->alignment < r.meta->alignment; });

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

    // layout must be stored by ComponentId
    std::sort(archetype.layout.begin(), archetype.layout.end(), [](const auto& l, const auto& r) noexcept { return l.meta->id < r.meta->id; });
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

bool up::World::_matchArchetype(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    Archetype const& archetypeData = *_archetypes[archetypeIndex];

    // FIXME: handle Archetypes that have multiple copies of the same component
    for (ComponentId id : components) {
        if (find(archetypeData.layout, id, {}, [](auto const& layout) noexcept { return layout.meta->id; }) == archetypeData.layout.end()) {
            return false;
        }
    }
    return true;
}

bool up::World::_matchArchetypeExact(uint32 archetypeIndex, view<ComponentMeta const*> components) const noexcept {
    Archetype const& archetypeData = *_archetypes[archetypeIndex];

    if (components.size() != _archetypes[archetypeIndex]->layout.size()) {
        return false;
    }

    // FIXME: handle Archetypes that have multiple copies of the same component
    for (ComponentMeta const* meta : components) {
        if (find(archetypeData.layout, meta->id, {}, [](auto const& layout) noexcept { return layout.meta->id; }) == archetypeData.layout.end()) {
            return false;
        }
    }

    return true;
}

auto up::World::_findArchetypeIndex(view<ComponentMeta const*> components) noexcept -> up::uint32 {
    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_matchArchetypeExact(index, components)) {
            return index;
        }
    }

    auto arch = new_box<Archetype>();
    _archetypes.push_back(std::move(arch));
    uint32 archetypeIndex = static_cast<uint32>(_archetypes.size() - 1);
    _calculateLayout(archetypeIndex, components);

    return archetypeIndex;
}

void up::World::_selectRaw(view<ComponentId> components, delegate_ref<RawSelectSignature> callback) const {
    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_matchArchetype(index, components)) {
            _selectChunksRaw(index, components, callback);
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

    EntityId entity = _allocateEntityId(archetypeIndex, entityIndex);

    auto chunkIndex = entityIndex / archetype.perChunk;
    auto subIndex = entityIndex % archetype.perChunk;

    UP_ASSERT(chunkIndex <= archetype.chunks.size());

    if (chunkIndex == archetype.chunks.size()) {
        archetype.chunks.push_back(_allocateChunk());
    }

    Chunk& chunk = *archetype.chunks[chunkIndex];
    ++chunk.header.count;

    *chunk.entity(subIndex) = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentMeta const& meta = *components[index];

        auto layoutIter = find(archetype.layout, meta.id, {}, [](auto const& layout) noexcept { return layout.meta->id; });
        UP_ASSERT(layoutIter != archetype.layout.end());
        
        void* rawPointer = chunk.pointer(*layoutIter, subIndex);

        meta.copy(rawPointer, data[index]);
    }

    uint32 entityMappingIndex = getEntityMappingIndex(entity);
    _entityMapping[entityMappingIndex].archetype = archetypeIndex;
    _entityMapping[entityMappingIndex].index = entityIndex;

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
    Layout const* layouts[64];
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));

    Archetype const& archetype = *_archetypes[archetypeIndex];

    for (size_t i = 0; i < components.size(); ++i) {
        layouts[i] = find(archetype.layout, components[i], {}, [](auto const& layout) noexcept { return layout.meta->id; });
        UP_ASSERT(layouts[i] != archetype.layout.end());
    }

    for (box<Chunk> const& chunk : _archetypes[archetypeIndex]->chunks) {
        EntityId const* entities = chunk->entity(0);

        for (size_t i = 0; i < components.size(); ++i) {
            pointers[i] = chunk->pointer(*layouts[i], 0);
        }

        callback(static_cast<size_t>(chunk->header.count), entities, view<void*>(pointers).first(components.size()));
    }
}

void* up::World::_getComponentPointer(up::uint32 archetypeIndex, up::uint32 entityIndex, ComponentId component) const noexcept {
    Archetype& archetype = *_archetypes[archetypeIndex];

    auto layoutIter = find(archetype.layout, component, {}, [](auto const& layout) noexcept { return layout.meta->id; });
    UP_ASSERT(layoutIter != archetype.layout.end());

    auto chunkIndex = entityIndex / archetype.perChunk;
    auto subIndex = entityIndex % archetype.perChunk;

    return archetype.chunks[chunkIndex]->pointer(*layoutIter, entityIndex);
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
