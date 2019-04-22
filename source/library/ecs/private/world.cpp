// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"

up::World::World() = default;
up::World::~World() = default;

void up::World::deleteEntity(EntityId entity) noexcept {
    uint32 index = getEntityIndex(entity);

    UP_ASSERT(index < _entityMapping.size());

    EntityMapping const& mapping = _entityMapping[index];

    UP_ASSERT(mapping.generation == getEntityGeneration(entity));

    _deleteEntity(mapping.archetype, mapping.index);
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> Archetype const* {
    return _archetypes[_findArchetypeIndex(components)].get();
}

auto up::World::archetypes() const noexcept -> view<rc<Archetype>> {
    return _archetypes;
}

bool up::World::_matchArchetype(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    // FIXME: handle Archetypes that have multiple copies of the same component
    Archetype const& archetypeData = *_archetypes[archetypeIndex];
    for (ComponentId component : components) {
        if (find(archetypeData._layout, component, {}, &Archetype::Layout::component) == archetypeData._layout.end()) {
            return false;
        }
    }
    return true;
}

bool up::World::_matchArchetypeExact(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
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

    auto arch = new_shared<Archetype>(components);
    _archetypes.push_back(std::move(arch));
    return static_cast<uint32>(_archetypes.size() - 1);
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

    EntityId entity = _allocateEntityId();
    uint32 archetypeIndex = _findArchetypeIndex(components);

    uint32 index = _createArchetypeEntity(archetypeIndex, entity, components, data);

    _entityMapping.push_back({getEntityGeneration(entity), archetypeIndex, index});

    return entity;
}

void* up::World::_getComponentPointer(EntityId entity, ComponentId component) noexcept {
    uint32 index = getEntityIndex(entity);
    if (index >= _entityMapping.size()) {
        return nullptr;
    }

    EntityMapping const& mapping = _entityMapping[index];
    if (mapping.generation != getEntityGeneration(entity)) {
        return nullptr;
    }

    // FIXME: bounds check
    return _getComponentPointer(mapping.archetype, mapping.index, component);
}

void up::World::_selectArchetype(up::uint32 archetypeIndex, view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));

    for (size_t i = 0; i < components.size(); ++i) {
        pointers[i] = _getComponentPointer(archetypeIndex, 0, components[i]);
    }

    callback(static_cast<size_t>(_archetypes[archetypeIndex]->_count), view<void*>(pointers).first(components.size()));
}

void* up::World::_getComponentPointer(up::uint32 archetypeIndex, up::uint32 entityIndex, ComponentId component) const noexcept {
    Archetype& archetype = *_archetypes[archetypeIndex];

    auto layoutIter = find(archetype._layout, component, {}, [](auto const& layout) noexcept { return layout.component; });
    UP_ASSERT(layoutIter != archetype._layout.end());
    ComponentInfo info(component);

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    return archetype._chunks[chunkIndex]->data + layoutIter->offset + entityIndex * info.size;
}

void up::World::_deleteEntity(up::uint32 archetypeIndex, up::uint32 entityIndex) noexcept {
    Archetype& archetype = *_archetypes[archetypeIndex];

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    UP_ASSERT(chunkIndex < archetype._chunks.size());
    UP_ASSERT(subIndex < archetype._perChunk);

    auto lastChunkIndex = archetype._chunks.size() - 1;
    auto lastSubIndex = archetype._chunks[lastChunkIndex]->header.count - 1;
    EntityId lastEntity = archetype._entities.back();

    if (lastChunkIndex != chunkIndex || lastSubIndex != subIndex) {
        archetype._entities[entityIndex] = lastEntity;

        for (auto const& layout : archetype._layout) {
            ComponentInfo info(layout.component);
            void* pointer = archetype._chunks[chunkIndex]->data + layout.offset + subIndex * info.size;
            void const* lastPointer = archetype._chunks[lastChunkIndex]->data + layout.offset + lastSubIndex * info.size;
            std::memcpy(pointer, lastPointer, info.size);
        }
    }

    if (--archetype._chunks[lastChunkIndex]->header.count == 0) {
        _chunkPool.push_back(std::move(archetype._chunks[lastChunkIndex]));
        archetype._chunks.pop_back();
    }
    archetype._entities.pop_back();

    _entityMapping[getEntityIndex(lastEntity)].index = entityIndex;
}

auto up::World::_createArchetypeEntity(up::uint32 archetypeIndex, EntityId entity, view<ComponentId> componentIds, view<void const*> componentData) noexcept -> up::uint32 {
    Archetype& archetype = *_archetypes[archetypeIndex];

    UP_ASSERT(componentData.size() == archetype._layout.size());
    UP_ASSERT(componentIds.size() == componentData.size());

    uint32 entityIndex = archetype._count++;

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    UP_ASSERT(chunkIndex <= archetype._chunks.size());

    archetype._entities.push_back(entity);
    if (chunkIndex == archetype._chunks.size()) {
        archetype._chunks.push_back(_allocateChunk());
    }

    ++archetype._chunks[chunkIndex]->header.count;

    for (uint32 index = 0; index != componentIds.size(); ++index) {
        ComponentId componentId = componentIds[index];
        ComponentInfo info(componentId);

        auto layoutIter = find(archetype._layout, componentId, {}, [](auto const& layout) noexcept { return layout.component; });
        UP_ASSERT(layoutIter != archetype._layout.end());

        void* rawPointer = archetype._chunks[chunkIndex]->data + layoutIter->offset + subIndex * info.size;
        UP_ASSERT(rawPointer >= archetype._chunks[chunkIndex]->data);
        UP_ASSERT(rawPointer <= archetype._chunks[chunkIndex]->data + sizeof(EntityChunk::Payload) - info.size);

        std::memcpy(rawPointer, componentData[index], info.size);
    }

    return entityIndex;
}

auto up::World::_allocateEntityId() noexcept -> EntityId {
    if (_freeEntityHead == static_cast<decltype(_freeEntityHead)>(-1)) {
        // nothing in free list
        uint32 firstGeneration = 1;
        _entityMapping.push_back({0, firstGeneration, 0});
        return makeEntityId(static_cast<uint32>(_entityMapping.size()), firstGeneration);
    }

    uint32 index = _freeEntityHead;
    _freeEntityHead = _entityMapping[index].index;
    return makeEntityId(index, ++_entityMapping[index].generation);
}

void up::World::_recycleEntityId(EntityId entity) noexcept {
    uint32 index = getEntityIndex(entity);

    ++_entityMapping[index].generation;
    _entityMapping[index].index = _freeEntityHead;

    _freeEntityHead = index;
}

auto up::World::_allocateChunk() -> box<EntityChunk> {
    if (!_chunkPool.empty()) {
        auto chunk = std::move(_chunkPool.back());
        _chunkPool.pop_back();
        return chunk;
    }

    return new_box<EntityChunk>();
}

void up::World::_recycleChunk(box<EntityChunk> chunk) {
    _chunkPool.push_back(std::move(chunk));
}
