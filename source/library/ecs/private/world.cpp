// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"

up::World::World() = default;
up::World::~World() = default;

void up::World::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    for (rc<Archetype> const& archetype : _archetypes) {
        if (archetype->matches(components)) {
            archetype->unsafeSelect(components, callback);
        }
    }
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> rc<Archetype> {
    return _archetypes[_findArchetypeIndex(components)];
}

auto up::World::_findArchetypeIndex(view<ComponentId> components) noexcept -> uint32 {
    uint32 index = 0;
    for (; index != _archetypes.size(); ++index) {
        if (_archetypes[index]->matchesExact(components)) {
            return index;
        }
    }

    auto arch = new_shared<Archetype>(components);
    _archetypes.push_back(std::move(arch));
    return index;
}

void* up::World::getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept {
    uint32 index = getEntityIndex(entity);
    if (index >= _entities.size()) {
        return nullptr;
    }

    EntityMapping const& mapping = _entities[index];
    if (mapping.generation != getEntityGeneration(entity)) {
        return nullptr;
    }

    // FIXME: bounds check
    return _archetypes[mapping.archetype]->unsafeComponentPointer(mapping.index, component);
}
