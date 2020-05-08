// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/_detail/ecs_context.h"
#include "potato/ecs/archetype.h"
#include <potato/spud/find.h>
#include <potato/runtime/assertion.h>
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

up::World::World(_detail::EcsContext& context) : _context(context) {}

up::World::~World() = default;

void up::World::deleteEntity(EntityId entity) noexcept {
    if (_entityMapper.isValid(entity)) {
        _deleteEntity(entity);
        _entityMapper.recycle(entity);
    }
}

auto up::World::chunksOf(ArchetypeId arch) const noexcept -> view<Chunk*> {
    auto const* desc = _archetypeMapper.getArchetype(arch);
    if (desc != nullptr) {
        return _chunks.subspan(desc->chunksOffset, desc->chunksLength);
    }
    return {};
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entityMapper.tryParse(entity); success) {
        auto const layout = _archetypeMapper.layoutOf(archetypeId);

        if (auto const row = findRowDesc(layout, component); row != nullptr) {
            auto& chunk = *_getChunk(archetypeId, chunkIndex);
            return chunk.data + row->offset + row->width * index;
        }
    }
    return nullptr;
}

void up::World::_deleteEntity(EntityId entity) {
    auto [archetypeId, chunkIndex, index] = _entityMapper.parse(entity);

    Chunk* chunk = _getChunk(archetypeId, chunkIndex);

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    //
    auto const lastIndex = chunk->header.entities - 1;
    if (index != lastIndex) {
        auto const movedEntity = static_cast<EntityId*>(static_cast<void*>(chunk->data))[index] = static_cast<EntityId*>(static_cast<void*>(chunk->data))[lastIndex];
        _moveTo(archetypeId, *chunk, index, *chunk, lastIndex);
        _entityMapper.setIndex(movedEntity, chunkIndex, index);
    }

    _destroyAt(archetypeId, *chunk, lastIndex);

    // if this was the last entity, deallocate the whole chunk
    //
    if (--chunk->header.entities == 0) {
        _removeChunk(archetypeId, chunkIndex);
        _context.recycle(chunk);
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    ComponentMeta const* const meta = _context.findById(componentId);
    UP_ASSERT(meta != nullptr);

    if (auto [success, archetypeId, chunkIndex, index] = _entityMapper.tryParse(entityId); success) {
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWithout(archetypeId, meta, static_cast<uint32>(_chunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* oldChunk = _getChunk(archetypeId, chunkIndex);
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *oldChunk, index);

        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;

        // remove old entity (must be gone before remap)
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        _entityMapper.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

void up::World::addComponentDefault(EntityId entityId, ComponentMeta const& componentMeta) {
    if (auto [success, archetypeId, chunkIndex, index] = _entityMapper.tryParse(entityId); success) {
        // find the target archetype and allocate an entry in it
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWith(archetypeId, &componentMeta, static_cast<uint32>(_chunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* chunk = _getChunk(archetypeId, chunkIndex);
        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        _constructAt(newArchetype, newChunk, newIndex, componentMeta.id);

        // remove old entity (must be gone before remap)
        //
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        //
        _entityMapper.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const& componentMeta, void const* componentData) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entityMapper.tryParse(entityId); success) {
        // find the target archetype and allocate an entry in it
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWith(archetypeId, &componentMeta, static_cast<uint32>(_chunks.size()));
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

        auto* chunk = _getChunk(archetypeId, chunkIndex);
        static_cast<EntityId*>(static_cast<void*>(newChunk.data))[newIndex] = entityId;
        _moveTo(newArchetype, newChunk, newIndex, archetypeId, *chunk, index);
        _copyTo(newArchetype, newChunk, newIndex, componentMeta.id, componentData);

        // remove old entity (must be gone before remap)
        //
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        //
        _entityMapper.setArchetype(entityId, newArchetype, newChunkIndex, newIndex);
    }
}

auto up::World::_createEntityRaw(view<ComponentMeta const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    ArchetypeId newArchetype = _archetypeMapper.acquireArchetype(components, static_cast<uint32>(_chunks.size()));
    auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(newArchetype);

    // Allocate EntityId
    auto const entity = _entityMapper.allocate(newArchetype, newChunkIndex, newIndex);
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
        chunk = _context.allocate(archetype);
        _addChunk(archetype, chunk);
    }

    uint16 index = chunk->header.entities++;
    return {*chunk, static_cast<uint16>(chunkIndex), index};
}

void up::World::_moveTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ArchetypeId srcArch, Chunk& srcChunk, int srcIndex) {
    auto const srcLayout = _archetypeMapper.layoutOf(srcArch);
    for (ChunkRowDesc const& row : _archetypeMapper.layoutOf(destArch)) {
        if (auto const srcRow = findRowDesc(srcLayout, row.component); srcRow != nullptr) {
            row.meta->ops.moveAssign(destChunk.data + row.offset + row.width * destIndex, srcChunk.data + srcRow->offset + srcRow->width * srcIndex);
        }
    }
}

void up::World::_moveTo(ArchetypeId arch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex) {
    for (ChunkRowDesc const& layout : _archetypeMapper.layoutOf(arch)) {
        layout.meta->ops.moveAssign(destChunk.data + layout.offset + layout.width * destIndex, srcChunk.data + layout.offset + layout.width * srcIndex);
    }
}

void up::World::_copyTo(ArchetypeId destArch, Chunk& destChunk, int destIndex, ComponentId srcComponent, void const* srcData) {
    auto const destRow = findRowDesc(_archetypeMapper.layoutOf(destArch), srcComponent);
    destRow->meta->ops.copyConstruct(destChunk.data + destRow->offset + destRow->width * destIndex, srcData);
}

void up::World::_constructAt(ArchetypeId arch, Chunk& chunk, int index, ComponentId component) {
    auto const row = findRowDesc(_archetypeMapper.layoutOf(arch), component);
    row->meta->ops.defaultConstruct(chunk.data + row->offset + row->width * index);
}

void up::World::_destroyAt(ArchetypeId arch, Chunk& chunk, int index) {
    for (ChunkRowDesc const& layout : _archetypeMapper.layoutOf(arch)) {
        layout.meta->ops.destruct(chunk.data + layout.offset + layout.width * index);
    }
}

auto up::World::_addChunk(ArchetypeId arch, Chunk* chunk) -> int {
    UP_ASSERT(arch == chunk->header.archetype);

    Archetype* archData = _archetypeMapper.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    int const lastIndex = archData->chunksLength;

    chunk->header.capacity = archData->maxEntitiesPerChunk;

    _chunks.insert(_chunks.begin() + archData->chunksOffset + archData->chunksLength, chunk);
    ++archData->chunksLength;

    for (auto& updateArch : _archetypeMapper.archetypes().subspan(to_underlying(arch) + 1)) {
        ++updateArch.chunksOffset;
    }

    return lastIndex;
}

void up::World::_removeChunk(ArchetypeId arch, int chunkIndex) noexcept {
    Archetype* archData = _archetypeMapper.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    _chunks.erase(_chunks.begin() + archData->chunksOffset + chunkIndex);
    --archData->chunksLength;

    for (auto& updateArch : _archetypeMapper.archetypes().subspan(to_underlying(arch) + 1)) {
        --updateArch.chunksOffset;
    }
}

auto up::World::_getChunk(ArchetypeId arch, int chunkIndex) const noexcept -> Chunk* {
    Archetype const* archData = _archetypeMapper.getArchetype(arch);
    return archData != nullptr ? _chunks[archData->chunksOffset + chunkIndex] : nullptr;
}
