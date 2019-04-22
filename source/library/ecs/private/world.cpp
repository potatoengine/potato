// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"

up::World::World() : _state(new_box<EntityDomain>()) {}
up::World::~World() = default;

void up::World::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    for (rc<Archetype> const& archetype : _state->archetypes) {
        if (archetype->matches(components)) {
            archetype->unsafeSelect(components, callback);
        }
    }
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> Archetype const* {
    return _state->archetypes[_findArchetypeIndex(components)].get();
}

auto up::World::archetypes() const noexcept -> view<rc<Archetype>> {
    return _state->archetypes;
}

auto up::World::_findArchetypeIndex(view<ComponentId> components) noexcept -> up::uint32 {
    uint32 index = 0;
    for (; index != _state->archetypes.size(); ++index) {
        if (_state->archetypes[index]->matchesExact(components)) {
            return index;
        }
    }

    auto arch = new_shared<Archetype>(*_state, components);
    _state->archetypes.push_back(std::move(arch));
    return index;
}

auto up::World::unsafeCreateEntity(view<ComponentId> components, view<void const*> data) -> EntityId {
    UP_ASSERT(components.size() == data.size());

    EntityId entity = _state->allocateEntityId();
    uint32 archetype = _findArchetypeIndex(components);

    uint32 index = _state->archetypes[archetype]->unsafeAllocate(entity, components, data);

    _state->entityMapping.push_back({getEntityGeneration(entity), archetype, index});

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
    return _state->archetypes[mapping.archetype]->unsafeComponentPointer(mapping.index, component);
}
