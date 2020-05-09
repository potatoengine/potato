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
    auto const archIndex = to_underlying(arch);
    if (archIndex < 0 || archIndex >= _archetypeChunkRanges.size()) {
        return {};
    }
    auto const& range = _archetypeChunkRanges[archIndex];
    return _chunks.subspan(range.offset, range.length);
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
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWithout(archetypeId, meta);
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
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWith(archetypeId, &componentMeta);
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
        ArchetypeId newArchetype = _archetypeMapper.acquireArchetypeWith(archetypeId, &componentMeta);
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

    ArchetypeId newArchetype = _archetypeMapper.acquireArchetype(components);
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

void up::World::_addChunk(ArchetypeId arch, Chunk* chunk) {
    UP_ASSERT(arch == chunk->header.archetype);

    Archetype* archData = _archetypeMapper.getArchetype(arch);
    UP_ASSERT(archData != nullptr);

    auto const archIndex = to_underlying(arch);
    UP_ASSERT(archIndex >= 0);

    if (archIndex >= _archetypeChunkRanges.size()) {
        _archetypeChunkRanges.resize(archIndex + 1, {static_cast<uint32>(_chunks.size()), 0});
    }

    auto& range = _archetypeChunkRanges[archIndex];
    chunk->header.capacity = archData->maxEntitiesPerChunk;

    _chunks.insert(_chunks.begin() + range.offset + range.length, chunk);
    ++range.length;

    for (auto& updateRange : _archetypeChunkRanges.subspan(archIndex + 1)) {
        ++updateRange.offset;
    }
}

void up::World::_removeChunk(ArchetypeId arch, int chunkIndex) noexcept {
    auto const archIndex = to_underlying(arch);
    UP_ASSERT(archIndex >= 0 && archIndex < _archetypeChunkRanges.size());

    auto& range = _archetypeChunkRanges[archIndex];
    UP_ASSERT(chunkIndex >= 0 && chunkIndex < static_cast<int>(range.length));

    _chunks.erase(_chunks.begin() + range.offset + chunkIndex);
    --range.length;

    for (auto& updateRange : _archetypeChunkRanges.subspan(archIndex + 1)) {
        --updateRange.offset;
    }
}

auto up::World::_getChunk(ArchetypeId arch, int chunkIndex) const noexcept -> Chunk* {
    auto const archIndex = to_underlying(arch);
    if (archIndex < 0 || archIndex >= _archetypeChunkRanges.size()) {
        return nullptr;
    }
    auto& range = _archetypeChunkRanges[archIndex];
    UP_ASSERT(chunkIndex >= 0 && chunkIndex < static_cast<int>(range.length));
    return _chunks[range.offset + chunkIndex];
}
