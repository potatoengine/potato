// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/world.h"
#include "potato/ecs/archetype.h"

up::World::World() = default;
up::World::~World() = default;

void up::World::unsafeSelect(Query const& query, delegate_ref<SelectSignature> callback) const noexcept {
    for (rc<Archetype> const& archetype : _archetypes) {
        if (archetype->matches(query.components())) {
            archetype->unsafeSelect(query, callback);
        }
    }
}
