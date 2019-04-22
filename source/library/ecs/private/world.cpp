// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"

up::World::World() : _state(new_box<EntityDomain>()) {}
up::World::~World() = default;

void up::World::deleteEntity(EntityId entity) noexcept {
    uint32 index = getEntityIndex(entity);

    UP_ASSERT(index < _state->entityMapping.size());

    EntityMapping const& mapping = _state->entityMapping[index];

    UP_ASSERT(mapping.generation == getEntityGeneration(entity));

    unsafeRemoveEntity(mapping.archetype, mapping.index);
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> Archetype const* {
    return _state->archetypes[_findArchetypeIndex(components)].get();
}

auto up::World::archetypes() const noexcept -> view<rc<Archetype>> {
    return _state->archetypes;
}

bool up::World::unsafeMatch(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    // FIXME: handle Archetypes that have multiple copies of the same component
    Archetype const& archetypeData = *_state->archetypes[archetypeIndex];
    for (ComponentId component : components) {
        if (find(archetypeData._layout, component, {}, &Archetype::Layout::component) == archetypeData._layout.end()) {
            return false;
        }
    }
    return true;
}

bool up::World::unsafeExactMatch(uint32 archetypeIndex, view<ComponentId> components) const noexcept {
    if (components.size() != _state->archetypes[archetypeIndex]->_layout.size()) {
        return false;
    }

    return unsafeMatch(archetypeIndex, components);
}

auto up::World::_findArchetypeIndex(view<ComponentId> components) noexcept -> up::uint32 {
    for (uint32 index = 0; index != _state->archetypes.size(); ++index) {
        if (unsafeExactMatch(index, components)) {
            return index;
        }
    }

    auto arch = new_shared<Archetype>(*_state, components);
    _state->archetypes.push_back(std::move(arch));
    return static_cast<uint32>(_state->archetypes.size() - 1);
}

void up::World::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    for (uint32 index = 0; index != _state->archetypes.size(); ++index) {
        if (unsafeMatch(index, components)) {
            unsafeSelect(index, components, callback);
        }
    }
}

auto up::World::unsafeCreateEntity(view<ComponentId> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    EntityId entity = _state->allocateEntityId();
    uint32 archetypeIndex = _findArchetypeIndex(components);

    uint32 index = unsafeAllocate(archetypeIndex, entity, components, data);

    _state->entityMapping.push_back({getEntityGeneration(entity), archetypeIndex, index});

    return entity;
}

void* up::World::unsafeGetComponentSlow(EntityId entity, ComponentId component) noexcept {
    uint32 index = getEntityIndex(entity);
    if (index >= _state->entityMapping.size()) {
        return nullptr;
    }

    EntityMapping const& mapping = _state->entityMapping[index];
    if (mapping.generation != getEntityGeneration(entity)) {
        return nullptr;
    }

    // FIXME: bounds check
    return unsafeComponentPointer(mapping.archetype, mapping.index, component);
}

void up::World::unsafeSelect(up::uint32 archetypeIndex, view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));

    for (size_t i = 0; i < components.size(); ++i) {
        pointers[i] = unsafeComponentPointer(archetypeIndex, 0, components[i]);
    }

    callback(static_cast<size_t>(_state->archetypes[archetypeIndex]->_count), view<void*>(pointers).first(components.size()));
}

void* up::World::unsafeComponentPointer(up::uint32 archetypeIndex, up::uint32 entityIndex, ComponentId component) const noexcept {
    Archetype& archetype = *_state->archetypes[archetypeIndex];

    auto layoutIter = find(archetype._layout, component, {}, [](auto const& layout) noexcept { return layout.component; });
    UP_ASSERT(layoutIter != archetype._layout.end());
    ComponentInfo info(component);

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    return archetype._chunks[chunkIndex]->data + layoutIter->offset + entityIndex * info.size;
}

void up::World::unsafeRemoveEntity(up::uint32 archetypeIndex, up::uint32 entityIndex) noexcept {
    Archetype& archetype = *_state->archetypes[archetypeIndex];

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
        _state->returnChunk(std::move(archetype._chunks[lastChunkIndex]));
        archetype._chunks.pop_back();
    }
    archetype._entities.pop_back();

    _state->entityMapping[getEntityIndex(lastEntity)].index = entityIndex;
}

auto up::World::unsafeAllocate(up::uint32 archetypeIndex, EntityId entity, view<ComponentId> componentIds, view<void const*> componentData) noexcept -> up::uint32 {
    Archetype& archetype = *_state->archetypes[archetypeIndex];

    UP_ASSERT(componentData.size() == archetype._layout.size());
    UP_ASSERT(componentIds.size() == componentData.size());

    uint32 entityIndex = archetype._count++;

    auto chunkIndex = entityIndex / archetype._perChunk;
    auto subIndex = entityIndex % archetype._perChunk;

    UP_ASSERT(chunkIndex <= archetype._chunks.size());

    archetype._entities.push_back(entity);
    if (chunkIndex == archetype._chunks.size()) {
        archetype._chunks.push_back(archetype._domain.allocateChunk());
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
