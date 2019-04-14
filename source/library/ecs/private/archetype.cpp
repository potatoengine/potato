// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"

up::Archetype::Archetype(std::initializer_list<ComponentId> comps) noexcept : _components(comps) {}
up::Archetype::Archetype(vector<ComponentId> comps) noexcept : _components(std::move(comps)) {}

bool up::Archetype::matches(Query const& query) const noexcept {
    for (ComponentId comp : query.components()) {
        if (find(_components, comp) == _components.end()) {
            return false;
        }
    }
    return true;
}
