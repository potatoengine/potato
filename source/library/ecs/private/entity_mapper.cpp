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

    static constexpr auto makeMapped(uint32 generation, uint32 archetypeIndex, uint32 entityIndex) -> uint64 {
        return (static_cast<uint64>(generation & 0xFFFF) << 48) | (static_cast<uint64>(archetypeIndex & 0xFFFF) << 32) | static_cast<uint64>(entityIndex);
    }

    static constexpr auto getMappedIndex(uint64 mapping) noexcept -> uint32 {
        return static_cast<uint32>(mapping & 0xFFFFFFFF);
    }

    static constexpr auto getMappedGeneration(uint64 mapping) noexcept -> uint32 {
        return static_cast<uint32>(mapping >> 48);
    }

    static constexpr auto getMappedArchetype(uint64 mapping) noexcept -> uint32 {
        return static_cast<uint32>(mapping >> 32) & 0xFFFF;
    }
} // namespace up

auto up::EntityMapper::allocate(ArchetypeId archetype, uint32 index) -> EntityId {
    // if there's a free ID, recycle it
    if (_freeEntityHead != freeEntityIndex) {
        auto const mappingIndex = _freeEntityHead;
        auto const mapping = _entityMapping[mappingIndex];
        auto const generation = getMappedGeneration(mapping);

        _freeEntityHead = getMappedIndex(_entityMapping[mappingIndex]);
        
        _entityMapping[mappingIndex] = makeMapped(generation, to_underlying(archetype), index);

        return makeEntityId(mappingIndex, generation);
    }

    // there was no ID to recycle, so create a new one
    uint32 mappingIndex = static_cast<uint32>(_entityMapping.size());

    _entityMapping.push_back(makeMapped(1, to_underlying(archetype), index));

    return makeEntityId(mappingIndex, 1);
}

void up::EntityMapper::recycle(EntityId entity) noexcept {
    uint32 entityMappingIndex = getEntityMappingIndex(entity);

    _entityMapping[entityMappingIndex] = makeMapped(getEntityGeneration(entity) + 1, 0, _freeEntityHead);

    _freeEntityHead = entityMappingIndex;
}

auto up::EntityMapper::getIndex(EntityId entity) const noexcept -> uint32 {
    return getEntityMappingIndex(entity);
}

auto up::EntityMapper::tryParse(EntityId entity, ArchetypeId& out_archetype, uint32& out_index) const noexcept -> bool {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    if (entityMappingIndex >= _entityMapping.size()) {
        return false;
    }

    auto const mapped = _entityMapping[entityMappingIndex];

    if (getMappedGeneration(mapped) != getEntityGeneration(entity)) {
        return false;
    }

    out_archetype = ArchetypeId(getMappedArchetype(mapped));
    out_index = getMappedIndex(mapped);
    return true;
}

void up::EntityMapper::setArchetype(EntityId entity, ArchetypeId newArchetype, uint32 newIndex) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const mapped = _entityMapping[entityMappingIndex];
    _entityMapping[entityMappingIndex] = makeMapped(getMappedGeneration(mapped), to_underlying(newArchetype), newIndex);
}

void up::EntityMapper::setIndex(EntityId entity, uint32 newIndex) noexcept {
    auto const entityMappingIndex = getEntityMappingIndex(entity);
    auto const mapped = _entityMapping[entityMappingIndex];
    _entityMapping[entityMappingIndex] = makeMapped(getMappedGeneration(mapped), getMappedArchetype(mapped), newIndex);
}
