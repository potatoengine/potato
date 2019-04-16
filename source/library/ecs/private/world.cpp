// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"

up::World::World() = default;
up::World::~World() = default;

void up::World::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const noexcept {
    for (rc<Archetype> const& archetype : _archetypes) {
        if (archetype->matches(components)) {
            archetype->unsafeSelect(components, callback);
        }
    }
}

auto up::World::acquireArchetype(view<ComponentId> components) noexcept -> rc<Archetype> {
    for (rc<Archetype> const& archetype : _archetypes) {
        if (archetype->matchesExact(components)) {
            return archetype;
        }
    }

    auto arch = new_shared<Archetype>(components);
    _archetypes.push_back(arch);
    return arch;
}
