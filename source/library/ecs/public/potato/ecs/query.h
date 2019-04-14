// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/typelist.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/span.h"
#include "potato/ecs/component.h"

namespace up {
    /// A Query is used to select a list of Archetypes that provide a particular set of Components,
    /// used to efficiency enumerate all matching Entities.
    class Query {
    public:
        Query() noexcept = default;
        /*implicit*/ Query(std::initializer_list<ComponentId> comps) noexcept : _components(comps) {}
        explicit Query(vector<ComponentId> comps) noexcept : _components(std::move(comps)) {}

        view<ComponentId> components() const noexcept { return _components; }

    private:
        vector<ComponentId> _components;
    };
} // namespace up
