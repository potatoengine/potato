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

static constexpr void* chunkPointer(char* data, up::uint32 offset, up::uint32 index, up::uint32 alignment) noexcept {
    return data + offset + index * alignment;
}

up::World::World() = default;

up::World::~World() {
    while (_freeChunkHead != nullptr) {
        Chunk* chunk = _freeChunkHead;
        _freeChunkHead = _freeChunkHead->header.next;

        delete chunk;
    }
}

void up::World::_calculateLayout(uint32 archetypeIndex, view<ComponentId> components) {
    Archetype& archetype = *_archetypes[archetypeIndex];

    archetype._layout.resize(components.size());

    // we'll always include the EntityId is the layout, so include its size
    size_t size = sizeof(EntityId);

    for (size_t i = 0; i != components.size(); ++i) {
        ComponentInfo info(components[i]);
        archetype._layout[i].component = components[i];
        size = align(size, info.alignment);
        size += info.size;
    }
    UP_ASSERT(size <= sizeof(Chunk::data));

    if (size != 0) {
        // assign pointer offers by alignment
        std::sort(archetype._layout.begin(), archetype._layout.end(), [](const auto& l, const auto& r) noexcept { return ComponentInfo(l.component).alignment < ComponentInfo(l.component).alignment; });

        archetype._perChunk = static_cast<uint32>(sizeof(Chunk::Payload) / size);

        // we'll always include the EntityId is the layout, so include it as offset
        size_t offset = sizeof(EntityId) * archetype._perChunk;

        for (size_t i = 0; i != components.size(); ++i) {
            ComponentInfo info(components[i]);

            // align as required (requires alignment to be a power of 2)
            UP_ASSERT((info.alignment & (info.alignment - 1)) == 0);
            offset = align(offset, info.alignment);

            archetype._layout[i].offset = static_cast<uint32>(offset);

            offset += info.size * archetype._perChunk;
        }

        UP_ASSERT(offset <= sizeof(Chunk::data));
    }

    // layout must be stored by ComponentId
    std::sort(archetype._layout.begin(), archetype._layout.end(), [](const auto& l, const auto& r) noexcept { return l.component < r.component; });
}

void up::World::deleteEntity(EntityId entity) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);

    UP_ASSERT(entityMappingIndex < _entityMapping.size());

    Entity const& mapping = _entityMapping[entityMappingIndex];
    uint32 archetypeIndex = mapping.archetype;
    uint32 entityIndex = mapping.index;

    UP_ASSERT(mapping.generation == getEntityGeneration(entity));

    Archetype& archetype = *_archetypes[archetypeIndex];

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    UP_ASSERT(chunkIndex < archetype._chunks.size());

    auto lastChunkIndex = archetype._chunks.size() - 1;
    auto lastSubIndex = archetype._chunks[lastChunkIndex]->header.count - 1;

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    if (lastChunkIndex != chunkIndex || lastSubIndex != subIndex) {
        EntityId lastEntity = *reinterpret_cast<EntityId*>(chunkPointer(archetype._chunks[chunkIndex]->data, 0, subIndex, sizeof(EntityId))) =
            *reinterpret_cast<EntityId const*>(chunkPointer(archetype._chunks[lastChunkIndex]->data, 0, lastSubIndex, sizeof(EntityId)));
        _entityMapping[getEntityMappingIndex(lastEntity)].index = entityIndex;

        for (auto const& layout : archetype._layout) {
            ComponentInfo info(layout.component);
            void* pointer = chunkPointer(archetype._chunks[chunkIndex]->data, layout.offset, subIndex, info.size);
            void* lastPointer = chunkPointer(archetype._chunks[lastChunkIndex]->data, layout.offset, lastSubIndex, info.size);
            std::memcpy(pointer, lastPointer, info.size);
        }
    }

    if (--archetype._chunks[lastChunkIndex]->header.count == 0) {
        _recycleChunk(std::move(archetype._chunks[lastChunkIndex]));
        archetype._chunks.pop_back();
    }
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> Archetype const* {
    return _archetypes[_findArchetypeIndex(components)].get();
}

auto up::World::archetypes() const noexcept -> view<box<Archetype>> {
    return _archetypes;
}

bool up::World::_matchArchetype(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    Archetype const& archetypeData = *_archetypes[archetypeIndex];

    // FIXME: handle Archetypes that have multiple copies of the same component
    for (ComponentId component : components) {
        if (find(archetypeData._layout, component, {}, &Archetype::Layout::component) == archetypeData._layout.end()) {
            return false;
        }
    }
    return true;
}

bool up::World::_matchArchetypeExact(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    Archetype const& archetypeData = *_archetypes[archetypeIndex];

    if (components.size() != _archetypes[archetypeIndex]->_layout.size()) {
        return false;
    }

    return _matchArchetype(archetypeIndex, components);
}

auto up::World::_findArchetypeIndex(view<ComponentId> components) noexcept -> up::uint32 {
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

void up::World::_select(view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    for (uint32 index = 0; index != _archetypes.size(); ++index) {
        if (_matchArchetype(index, components)) {
            _selectArchetype(index, components, callback);
        }
    }
}

auto up::World::_createEntity(view<ComponentId> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    uint32 archetypeIndex = _findArchetypeIndex(components);

    Archetype& archetype = *_archetypes[archetypeIndex];

    UP_ASSERT(components.size() == archetype._layout.size());
    UP_ASSERT(components.size() == data.size());

    uint32 entityIndex = archetype._count++;

    EntityId entity = _allocateEntityId(archetypeIndex, entityIndex);

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    UP_ASSERT(chunkIndex <= archetype._chunks.size());

    if (chunkIndex == archetype._chunks.size()) {
        archetype._chunks.push_back(_allocateChunk());
    }

    Chunk& chunk = *archetype._chunks[chunkIndex];
    ++chunk.header.count;

    *reinterpret_cast<EntityId*>(chunkPointer(chunk.data, 0, subIndex, sizeof(EntityId))) = entity;

    for (uint32 index = 0; index != components.size(); ++index) {
        ComponentId componentId = components[index];
        ComponentInfo info(componentId);

        auto layoutIter = find(archetype._layout, componentId, {}, [](auto const& layout) noexcept { return layout.component; });
        UP_ASSERT(layoutIter != archetype._layout.end());

        
        void* rawPointer = chunkPointer(chunk.data, layoutIter->offset, subIndex, info.size);
        UP_ASSERT(rawPointer >= chunk.data);
        UP_ASSERT(rawPointer <= chunk.data + sizeof(Chunk::Payload) - info.size);

        std::memcpy(rawPointer, data[index], info.size);
    }

    uint32 entityMappingIndex = getEntityMappingIndex(entity);
    _entityMapping[entityMappingIndex].archetype = archetypeIndex;
    _entityMapping[entityMappingIndex].index = entityIndex;

    return entity;
}

void* up::World::_getComponentSlow(EntityId entity, ComponentId component) noexcept {
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

void up::World::_selectArchetype(up::uint32 archetypeIndex, view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));

    EntityId const* entities = reinterpret_cast<EntityId const*>(_archetypes[archetypeIndex]->_chunks.front()->data);

    for (size_t i = 0; i < components.size(); ++i) {
        pointers[i] = _getComponentPointer(archetypeIndex, 0, components[i]);
    }

    callback(static_cast<size_t>(_archetypes[archetypeIndex]->_count), entities, view<void*>(pointers).first(components.size()));
}

void* up::World::_getComponentPointer(up::uint32 archetypeIndex, up::uint32 entityIndex, ComponentId component) const noexcept {
    Archetype& archetype = *_archetypes[archetypeIndex];

    auto layoutIter = find(archetype._layout, component, {}, [](auto const& layout) noexcept { return layout.component; });
    UP_ASSERT(layoutIter != archetype._layout.end());
    ComponentInfo info(component);

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    return chunkPointer(archetype._chunks[chunkIndex]->data, layoutIter->offset, entityIndex, info.size);
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
