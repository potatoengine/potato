// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include "potato/ecs/registry.h"
#include "potato/ecs/component.h"
#include <potato/spud/find.h>

auto up::ComponentRegistry::defaultRegistry() noexcept -> ComponentRegistry& {
    static ComponentRegistry instance;
    return instance;
}


void up::ComponentRegistry::registerComponent(ComponentMeta const* meta) {
    _components.push_back(meta);
}

void up::ComponentRegistry::deregisterComponent(ComponentMeta const* meta) {
    auto const* it = find(_components, meta);
    if (it != _components.end()) {
        _components.erase(it);
    }
}

auto up::ComponentRegistry::findByName(string_view name) const noexcept -> ComponentMeta const* {
    auto const* it = find_if(_components, [name](ComponentMeta const* meta) noexcept {
        return meta->name == name;
    });
    return it != _components.end() ? *it : nullptr;
}
