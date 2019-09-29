// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/runtime/assertion.h>
#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/entity.h"
#include "potato/spud/find.h"
#include <algorithm>

namespace up {
    static auto findLayout(Archetype const& archetype, ComponentId component) noexcept -> ChunkRowDesc const* {
        for (ChunkRowDesc const& row : archetype.chunkLayout) {
            if (row.component == component) {
                return &row;
            }
        }
        return nullptr;
    }
} // namespace up

up::World::World() = default;

up::World::~World() = default;

auto up::World::version() const noexcept -> uint32 {
    return _archetypes.version();
}

auto up::World::archetypes() const noexcept -> view<Archetype> {
    return _archetypes.archetypes();
}

auto up::World::getArchetype(ArchetypeId arch) noexcept -> Archetype const* {
    return _archetypes.getArchetype(arch);
}

auto up::World::selectArchetypes(view<ComponentId> componentIds, span<int> offsetsBuffer, delegate_ref<SelectSignature> callback) const -> int {
    UP_ASSERT(componentIds.size() == offsetsBuffer.size());

    return _archetypes.selectArchetypes(componentIds, offsetsBuffer, callback);
}

void up::World::deleteEntity(EntityId entity) noexcept {
    if (_entities.isValid(entity)) {
        _deleteEntity(entity);
        _entities.recycle(entity);
    }
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entity); success) {
        auto& archetype = *_archetypes.getArchetype(archetypeId);

        if (auto const layout = findLayout(archetype, component); layout != nullptr) {
            auto& chunk = *archetype.chunks[chunkIndex];
            return chunk.data + layout->offset + layout->width * index;
        }
    }
    return nullptr;
}

void up::World::_deleteEntity(EntityId entity) {
    auto [archetypeId, chunkIndex, index] = _entities.parse(entity);

    Archetype& archetype = *_archetypes.getArchetype(archetypeId);
    Chunk& chunk = *archetype.chunks[chunkIndex];

    // Copy the last element over the to-be-removed element, so we don't have holes in our array
    //
    auto const lastIndex = chunk.header.entities;
    if (index != lastIndex) {
        _moveTo(archetype, chunk, index, chunk, lastIndex);

        auto const entityLayout = findLayout(archetype, getComponentId<Entity>());
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
        _chunks.recycle(std::move(archetype.chunks[chunkIndex]));

        // copy-and-pop should be safe, since chunk order doesn't matter
        //
        archetype.chunks[chunkIndex] = archetype.chunks.back();
        archetype.chunks.pop_back();
    }
}

void up::World::removeComponent(EntityId entityId, ComponentId componentId) noexcept {
    if (auto [success, archetypeId, chunkIndex, index] = _entities.tryParse(entityId); success) {
        auto& archetype = *_archetypes.getArchetype(archetypeId);
        auto& chunk = *archetype.chunks[chunkIndex];

        // figure out which layout entry is being removed
        auto iter = find(archetype.chunkLayout, componentId, {}, &ChunkRowDesc::component);
        if (iter == archetype.chunkLayout.end()) {
            return;
        }

        // construct the list of components that will exist in the new archetype
        ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
        auto out = copy(archetype.chunkLayout.begin(), iter, newComponentMetas, &ChunkRowDesc::meta);
        copy(iter + 1, archetype.chunkLayout.end(), out, &ChunkRowDesc::meta);
        span newComponentMetaSpan = span{newComponentMetas}.first(archetype.chunkLayout.size() - 1);

        // find the target archetype and allocate an entry in it
        ArchetypeComponentHasher hasher;
        for (auto const& row : archetype.chunkLayout) {
            hasher.hash(row.component);
        }
        auto const newArchetypeHash = hasher.finalize();

        Archetype const* newArchetype = _archetypes.findArchetype(newArchetypeHash);
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
        auto& chunk = *archetype.chunks[chunkIndex];

        // construct the list of components that will exist in the new archetype
        ComponentMeta const* newComponentMetas[ArchetypeComponentLimit];
        newComponentMetas[0] = componentMeta;
        copy(archetype.chunkLayout.begin(), archetype.chunkLayout.end(), newComponentMetas + 1, &ChunkRowDesc::meta);
        span newComponentMetaSpan = span{newComponentMetas}.first(archetype.chunkLayout.size() + 1);

        // find the target archetype and allocate an entry in it
        Archetype const* newArchetype = _archetypes.createArchetype(newComponentMetaSpan);
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

    Archetype const* newArchetype = _archetypes.findArchetype(archetypeHash);
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

auto up::World::_allocateEntity(Archetype const& archetype) -> AllocatedLocation {
    if (archetype.chunks.empty() || archetype.chunks.back()->header.entities == archetype.maxEntitiesPerChunk) {
        archetype.chunks.push_back(_chunks.allocate(archetype.id));
    }

    Chunk* chunk = archetype.chunks.back();
    uint16 chunkIndex = static_cast<uint16>(archetype.chunks.size() - 1);
    uint16 index = chunk->header.entities++;

    return {*chunk, chunkIndex, index};
}

void up::World::_moveTo(Archetype const& destArch, Chunk& destChunk, int destIndex, Archetype const& srcArch, Chunk& srcChunk, int srcIndex) {
    for (ChunkRowDesc const& layout : destArch.chunkLayout) {
        if (auto const srcLayout = findLayout(srcArch, layout.component); srcLayout != nullptr) {
            layout.meta->relocate(destChunk.data + layout.offset + layout.width * destIndex, srcChunk.data + srcLayout->offset + srcLayout->width * srcIndex);
        }
    }
}

void up::World::_moveTo(Archetype const& destArch, Chunk& destChunk, int destIndex, Chunk& srcChunk, int srcIndex) {
    for (ChunkRowDesc const& layout : destArch.chunkLayout) {
        layout.meta->relocate(destChunk.data + layout.offset + layout.width * destIndex, srcChunk.data + layout.offset + layout.width * srcIndex);
    }
}

void up::World::_copyTo(Archetype const& destArch, Chunk& destChunk, int destIndex, ComponentId srcComponent, void const* srcData) {
    auto const destLayout = findLayout(destArch, srcComponent);

    destLayout->meta->copy(destChunk.data + destLayout->offset + destLayout->width * destIndex, srcData);
}

void up::World::_destroyAt(Archetype const& arch, Chunk& chunk, int index) {
    for (ChunkRowDesc const& layout : arch.chunkLayout) {
        layout.meta->destroy(chunk.data + layout.offset + layout.width * index);
    }
}
