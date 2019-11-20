// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/entity.h"
#include "potato/spud/find.h"
#include <algorithm>

namespace up {
    static auto findLayout(view<ChunkRowDesc> layout, ComponentId component) noexcept -> ChunkRowDesc const* {
        for (ChunkRowDesc const& row : layout) {
            if (row.component == component) {
                return &row;
            }
        }
        return nullptr;
    }
} // namespace up

up::World::World() = default;

up::World::~World() = default;

void up::World::deleteEntity(EntityId entity) noexcept {
    if (_entities.isValid(entity)) {
        _deleteEntity(entity);
        _entities.recycle(entity);
    }
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entity); success) {
        auto& archetype = *_archetypes.getArchetype(archetypeId);
        auto const layout = _archetypes.layouts().subspan(archetype.layoutOffset, archetype.layoutLength);

        if (auto const row = findLayout(layout, component); row != nullptr) {
            auto& chunk = *_archetypes.chunks()[archetype.chunksOffset + chunkIndex];
            return chunk.data + row->offset + row->width * index;
        }
    }
    return nullptr;
}

void up::World::_deleteEntity(EntityId entity) {
    auto [archetypeId, chunkIndex, index] = _entities.parse(entity);

    Archetype& archetype = *_archetypes.getArchetype(archetypeId);
    Chunk& chunk = *_archetypes.chunks()[archetype.chunksOffset + chunkIndex];
    auto const layout = _archetypes.layouts().subspan(archetype.layoutOffset, archetype.layoutLength);

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    //
    auto const lastIndex = chunk.header.entities;
    if (index != lastIndex) {
        _moveTo(archetype, chunk, index, chunk, lastIndex);

        auto const entityLayout = findLayout(layout, getComponentId<Entity>());
        auto const movedEntity = static_cast<Entity const*>(static_cast<void*>(chunk.data + entityLayout->offset + entityLayout->width * lastIndex))->id;
        _entities.setIndex(movedEntity, chunkIndex, index);

        _destroyAt(archetype, chunk, lastIndex);
    }
    else {
        // We were already the last element, just clean up our memory
        //
        _destroyAt(archetype, chunk, lastIndex);
    }

    // if this was the last entity, deallocate the whole chunk
    //
    if (--chunk.header.entities == 0) {
        _chunks.recycle(&chunk);

        // copy-and-pop should be safe, since chunk order doesn't matter
        //
        _archetypes.removeChunk(archetype, chunkIndex);
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        auto& archetype = *_archetypes.getArchetype(archetypeId);
        auto& chunk = *_archetypes.chunks()[archetype.chunksOffset + chunkIndex];

        // figure out which layout entry is being removed
        auto const layout = _archetypes.layouts().subspan(archetype.layoutOffset, archetype.layoutLength);
        auto iter = find(layout, componentId, {}, &ChunkRowDesc::component);
        if (iter == layout.end()) {
            return;
        }

        // construct the list of components that will exist in the new archetype
        ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
        auto out = copy(layout.begin(), iter, newComponentMetas, &ChunkRowDesc::meta);
        copy(iter + 1, layout.end(), out, &ChunkRowDesc::meta);
        span newComponentMetaSpan = span{newComponentMetas}.first(layout.size() - 1);

        // find the target archetype and allocate an entry in it
        ArchetypeComponentHasher hasher;
        for (auto const& row : layout) {
            hasher.hash(row.component);
        }
        auto const newArchetypeHash = hasher.finalize();

        Archetype* newArchetype = _archetypes.findArchetype(newArchetypeHash);
        if (newArchetype == nullptr) {
            newArchetype = _archetypes.createArchetype(newComponentMetaSpan);
        }
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(*newArchetype);

        _moveTo(*newArchetype, newChunk, newIndex, archetype, chunk, index);

        // remove old entity (must be gone before remap)
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        _entities.setArchetype(entityId, newArchetype->id, newChunkIndex, newIndex);
    }
}

void up::World::_addComponentRaw(EntityId entityId, ComponentMeta const* componentMeta, void const* componentData) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        auto& archetype = *_archetypes.getArchetype(archetypeId);
        auto& chunk = *_archetypes.chunks()[archetype.chunksOffset + chunkIndex];
        auto const layout = _archetypes.layouts().subspan(archetype.layoutOffset, archetype.layoutLength);

        // construct the list of components that will exist in the new archetype
        ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
        newComponentMetas[0] = componentMeta;
        copy(layout.begin(), layout.end(), newComponentMetas + 1, &ChunkRowDesc::meta);
        span newComponentMetaSpan = span{newComponentMetas}.first(layout.size() + 1);

        // find the target archetype and allocate an entry in it
        Archetype* newArchetype = _archetypes.createArchetype(newComponentMetaSpan);
        auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(*newArchetype);

        _moveTo(*newArchetype, newChunk, newIndex, archetype, chunk, index);
        _copyTo(*newArchetype, newChunk, newIndex, componentMeta->id, componentData);

        // remove old entity (must be gone before remap)
        _deleteEntity(entityId);

        // update mapping (must be done after delete)
        _entities.setArchetype(entityId, newArchetype->id, newChunkIndex, newIndex);
    }
}

auto up::World::_createEntityRaw(view<ComponentMeta const*> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    ArchetypeComponentHasher hasher;
    for (auto const* meta : components) {
        hasher.hash(meta->id);
    }
    auto const archetypeHash = hasher.finalize();

    Archetype* newArchetype = _archetypes.findArchetype(archetypeHash);
    if (newArchetype == nullptr) {
        newArchetype = _archetypes.createArchetype(components);
    }
    auto [newChunk, newChunkIndex, newIndex] = _allocateEntity(*newArchetype);

    // Allocate EntityId
    auto const entity = _entities.allocate(newArchetype->id, newChunkIndex, newIndex);
    _copyTo(*newArchetype, newChunk, newIndex, getComponentId<Entity>(), &entity);

    for (auto index : sequence(components.size())) {
        _copyTo(*newArchetype, newChunk, newIndex, components[index]->id, data[index]);
    }

    return entity;
}

auto up::World::_allocateEntity(Archetype& archetype) -> AllocatedLocation {
    if (archetype.chunksLength == 0 || _archetypes.chunks()[archetype.chunksOffset + archetype.chunksLength - 1]->header.entities == archetype.maxEntitiesPerChunk) {
        _archetypes.addChunk(archetype, _chunks.allocate(archetype.id));
    }

    uint16 chunkIndex = static_cast<uint16>(archetype.chunksLength - 1);
    Chunk* chunk = _archetypes.chunks()[archetype.chunksOffset + chunkIndex];
    uint16 index = chunk->header.entities++;

    return {*chunk, chunkIndex, index};
}

void up::World::_moveTo(Archetype const& destArch, Chunk& destChunk, int destIndex, Archetype const& srcArch, Chunk& srcChunk, int srcIndex) {
    auto const srcLayout = _archetypes.layouts().subspan(srcArch.layoutOffset, srcArch.layoutLength);
    for (ChunkRowDesc const& row : _archetypes.layouts().subspan(destArch.layoutOffset, destArch.layoutLength)) {
        if (auto const srcRow = findLayout(srcLayout, row.component); srcRow != nullptr) {
            row.meta->relocate(destChunk.data + row.offset + row.width * destIndex, srcChunk.data + srcRow->offset + srcRow->width * srcIndex);
        }
    }
}

void up::World::_moveTo(Archetype const& destArch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex) {
    for (ChunkRowDesc const& layout : _archetypes.layouts().subspan(destArch.layoutOffset, destArch.layoutLength)) {
        layout.meta->relocate(destChunk.data + layout.offset + layout.width * destIndex, srcChunk.data + layout.offset + layout.width * srcIndex);
    }
}

void up::World::_copyTo(Archetype const& destArch, Chunk& destChunk, int destIndex, ComponentId srcComponent, void const* srcData) {
    auto const destLayout = _archetypes.layouts().subspan(destArch.layoutOffset, destArch.layoutLength);
    auto const destRow = findLayout(destLayout, srcComponent);

    destRow->meta->copy(destChunk.data + destRow->offset + destRow->width * destIndex, srcData);
}

void up::World::_destroyAt(Archetype const& arch, Chunk& chunk, int index) {
    for (ChunkRowDesc const& layout : _archetypes.layouts().subspan(arch.layoutOffset, arch.layoutLength)) {
        layout.meta->destroy(chunk.data + layout.offset + layout.width * index);
    }
}
