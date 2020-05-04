// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/spud/find.h"
#include <algorithm>

namespace up {
    static auto findRowDesc(view<ChunkRowDesc> layout, ComponentId component) noexcept -> ChunkRowDesc const* {
        for (ChunkRowDesc const& row : layout) {
            if (row.component == component) {
                return &row;
            }
        }
        return nullptr;
    }
} // namespace up

up::World::World(ComponentRegistry& registry) : _registry(registry) {}

up::World::~World() = default;

void up::World::deleteEntity(EntityId entity) noexcept {
    if (_entities.isValid(entity)) {
        _deleteEntity(entity);
        _entities.recycle(entity);
    }
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entity); success) {
        auto const layout = _archetypes.layoutOf(archetypeId);

        if (auto const row = findRowDesc(layout, component); row != nullptr) {
            auto& chunk = *_getArchetypeChunk(archetypeId, chunkIndex);
            return chunk.data + row->offset + row->width * index;
        }
    }
    return nullptr;
}

void up::World::_deleteEntity(EntityId entity) {
    auto [archetypeId, chunkIndex, index] = _entities.parse(entity);

    Chunk* chunk = _getArchetypeChunk(archetypeId, chunkIndex);

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    //
    auto const lastIndex = chunk->header.entities - 1;
    if (index != lastIndex) {
        auto const movedEntity = static_cast<EntityId*>(static_cast<void*>(chunk->data))[index] = static_cast<EntityId*>(static_cast<void*>(chunk->data))[lastIndex];
        _moveTo(archetypeId, *chunk, index, *chunk, lastIndex);
        _entities.setIndex(movedEntity, chunkIndex, index);
    }

    _destroyAt(archetypeId, *chunk, lastIndex);

    // if this was the last entity, deallocate the whole chunk
    //
    if (--chunk->header.entities == 0) {
        _removeArchetypeChunk(archetypeId, chunkIndex);
        _chunks.recycle(chunk);
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    ComponentMeta const* const meta = _registry.findById(componentId);
    UP_ASSERT(meta != nullptr);

    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        ArchetypeId newArchetype = _archetypes.acquireArchetypeWithout(archetypeId, meta, static_cast<uint32>(_archetypeChunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* oldChunk = _getArchetypeChunk(archetypeId, chunkIndex);
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *oldChunk, index);

        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;

        // remove old entity (must be gone before remap)
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        _entities.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

void up::World::addComponentDefault(EntityId entityId, ComponentMeta const& componentMeta) {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        // find the target archetype and allocate an entry in it
        ArchetypeId newArchetype = _archetypes.acquireArchetypeWith(archetypeId, &componentMeta, static_cast<uint32>(_archetypeChunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* chunk = _getArchetypeChunk(archetypeId, chunkIndex);
        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        _constructAt(newArchetype, newChunk, newIndex, componentMeta.id);

        // remove old entity (must be gone before remap)
        //
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        //
        _entities.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const& componentMeta, void const* componentData) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        // find the target archetype and allocate an entry in it
        ArchetypeId newArchetype = _archetypes.acquireArchetypeWith(archetypeId, &componentMeta, static_cast<uint32>(_archetypeChunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* chunk = _getArchetypeChunk(archetypeId, chunkIndex);
        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        _copyTo(newArchetype, newChunk, newIndex, componentMeta.id, componentData);

        // remove old entity (must be gone before remap)
        //
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        //
        _entities.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

auto up::World::_createEntityRaw(view<ComponentMeta const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    ArchetypeId newArchetype = _archetypes.acquireArchetype(components, static_cast<uint32>(_archetypeChunks.size()));
    auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

    // Allocate EntityId
    auto const entity = _entities.allocate(newArchetype, newChunkIndex, newIndex);
    static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entity;

    for (auto index : sequence(components.size())) {
        _copyTo(newArchetype, newChunk, newIndex, components[index]->id, data[index]);
    }

    return entity;
}

auto up::World::_allocateEntity(ArchetypeId archetype) -> AllocatedLocation {
    size_t chunkIndex = 0;
    Chunk* chunk = nullptr;

    auto const chunks = chunksOf(archetype);
    for (; chunkIndex != chunks.size(); ++chunkIndex) {
        chunk = chunks[chunkIndex];
        if (chunk->header.entities < chunk->header.capacity) {
            break;
        }
    }
    if (chunkIndex == chunks.size()) {
        chunk = _chunks.allocate(archetype);
        _addArchetypeChunk(archetype, chunk);
    }

    uint16 index = chunk->header.entities++;
    return {*chunk, static_cast<uint16>(chunkIndex), index};
}

void up::World::_moveTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ArchetypeId srcArch, Chunk& srcChunk, int srcIndex) {
    auto const srcLayout = _archetypes.layoutOf(srcArch);
    for (ChunkRowDesc const& row : _archetypes.layoutOf(destArch)) {
        if (auto const srcRow = findRowDesc(srcLayout, row.component); srcRow != nullptr) {
            row.meta->ops.moveAssign(destChunk.data + row.offset + row.width * destIndex, srcChunk.data + srcRow->offset + srcRow->width * srcIndex);
        }
    }
}

void up::World::_moveTo(ArchetypeId arch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex) {
    for (ChunkRowDesc const& layout : _archetypes.layoutOf(arch)) {
        layout.meta->ops.moveAssign(destChunk.data + layout.offset + layout.width * destIndex, srcChunk.data + layout.offset + layout.width * srcIndex);
    }
}

void up::World::_copyTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ComponentId srcComponent, void const* srcData) {
    auto const destRow = findRowDesc(_archetypes.layoutOf(destArch), srcComponent);
    destRow->meta->ops.copyConstruct(destChunk.data + destRow->offset + destRow->width * destIndex, srcData);
}

void up::World::_constructAt(ArchetypeId arch, Chunk& chunk, int index, ComponentId component) {
    auto const row = findRowDesc(_archetypes.layoutOf(arch), component);
    row->meta->ops.defaultConstruct(chunk.data + row->offset + row->width * index);
}

void up::World::_destroyAt(ArchetypeId arch, Chunk& chunk, int index) {
    for (ChunkRowDesc const& layout : _archetypes.layoutOf(arch)) {
        layout.meta->ops.destruct(chunk.data + layout.offset + layout.width * index);
    }
}

auto up::World::_addArchetypeChunk(ArchetypeId arch, Chunk* chunk) -> int {
    UP_ASSERT(arch == chunk->header.archetype);

    Archetype* archData = _archetypes.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    int const lastIndex = archData->chunksLength;

    chunk->header.capacity = archData->maxEntitiesPerChunk;

    _archetypeChunks.insert(_archetypeChunks.begin() + archData->chunksOffset + archData->chunksLength, chunk);
    ++archData->chunksLength;

    for (auto& updateArch : _archetypes.archetypes().subspan(to_underlying(arch) + 1)) {
        ++updateArch.chunksOffset;
    }

    return lastIndex;
}

void up::World::_removeArchetypeChunk(ArchetypeId arch, int chunkIndex) noexcept {
    Archetype* archData = _archetypes.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    _archetypeChunks.erase(_archetypeChunks.begin() + archData->chunksOffset + chunkIndex);
    --archData->chunksLength;

    for (auto& updateArch : _archetypes.archetypes().subspan(to_underlying(arch) + 1)) {
        --updateArch.chunksOffset;
    }
}
