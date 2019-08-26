// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/entity_mapper.h"
#include "potato/foundation/utility.h"

namespace up {
    static constexpr auto makeEntityId(uint32 mappingIndex, uint32 generation) noexcept -> EntityId {
        return static_cast<EntityId>((static_cast<uint64>(generation) << 32) | mappingIndex);
    }

    static constexpr auto getEntityMappingIndex(EntityId entity) noexcept -> uint32 {
        return static_cast<uint64>(entity) & 0xFFFFFFFF;
    }

    static constexpr auto getEntityGeneration(EntityId entity) noexcept -> uint32 {
        return static_cast<uint64>(entity) >> 32;
    }

    static constexpr auto makeMapped(uint16 generation, uint16 archetypeIndex, uint16 chunkIndex, uint16 entityIndex) -> uint64 {
        return (static_cast<uint64>(generation) << 48) | (static_cast<uint64>(archetypeIndex) << 32) | (static_cast<uint64>(chunkIndex) << 16) | static_cast<uint64>(entityIndex);
    }

    static constexpr auto getMappedChunk(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 16);
    }

    static constexpr auto getMappedIndex(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping);
    }

    static constexpr auto getMappedGeneration(uint64 mapping) noexcept -> uint32 {
        return static_cast<uint32>(mapping >> 48);
    }

    static constexpr auto getMappedArchetype(uint64 mapping) noexcept -> uint16 {
        return static_cast<uint16>(mapping >> 32);
    }

    static constexpr auto makeFreeEntry(uint16 generation, uint32 index) noexcept -> uint64 {
        return (static_cast<uint64>(generation) << 48) | static_cast<uint64>(index);
    }
} // namespace up

auto up::EntityMapper::allocate(ArchetypeId archetype, uint16 chunk, uint16 index) -> EntityId {
    // if there's a free ID, recycle it
    if (_freeEntityHead != freeEntityIndex) {
        auto const mappingIndex = _freeEntityHead;
        auto const mapping = _entityMapping[mappingIndex];
        auto const generation = getMappedGeneration(mapping);

        _freeEntityHead = getMappedIndex(_entityMapping[mappingIndex]);
        
        _entityMapping[mappingIndex] = makeMapped(generation, to_underlying(archetype), chunk, index);

        return makeEntityId(mappingIndex, generation);
    }

    // there was no ID to recycle, so create a new one
    uint32 const mappingIndex = static_cast<uint32>(_entityMapping.size());

    _entityMapping.push_back(makeMapped(1, to_underlying(archetype), chunk, index));

    return makeEntityId(mappingIndex, 1);
}

void up::EntityMapper::recycle(EntityId entity) noexcept {
    uint32 const entityMappingIndex = getEntityMappingIndex(entity);
    uint32 const newGeneration = getEntityGeneration(entity) + 1;

    _entityMapping[entityMappingIndex] = makeFreeEntry(newGeneration != 0 ? newGeneration : 1, _freeEntityHead);

    _freeEntityHead = entityMappingIndex;
}

auto up::EntityMapper::isValid(EntityId entity) const noexcept -> bool {
    auto const mappingIndex = getEntityMappingIndex(entity);
    return mappingIndex < _entityMapping.size() && getMappedGeneration(_entityMapping[mappingIndex]) == getEntityGeneration(entity);
}

auto up::EntityMapper::parse(EntityId entity) const noexcept -> ParseLocation {
    auto const mapped = _entityMapping[getEntityMappingIndex(entity)];

    return {ArchetypeId(getMappedArchetype(mapped)), getMappedChunk(mapped), getMappedIndex(mapped)};
}

auto up::EntityMapper::tryParse(EntityId entity) const noexcept -> TryParseLocation {
    if (isValid(entity)) {
        auto const mapped = _entityMapping[getEntityMappingIndex(entity)];

        return {true, ArchetypeId(getMappedArchetype(mapped)), getMappedChunk(mapped), getMappedIndex(mapped)};
    }
    return {false};
}

void up::EntityMapper::setArchetype(EntityId entity, ArchetypeId newArchetype, uint16 newChunk, uint16 newIndex) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const mapped = _entityMapping[entityMappingIndex];
    _entityMapping[entityMappingIndex] = makeMapped(getMappedGeneration(mapped), to_underlying(newArchetype), newChunk, newIndex);
}

void up::EntityMapper::setIndex(EntityId entity, uint16 newChunk, uint16 newIndex) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const mapped = _entityMapping[entityMappingIndex];
    _entityMapping[entityMappingIndex] = makeMapped(getMappedGeneration(mapped), getMappedArchetype(mapped), newChunk, newIndex);
}
