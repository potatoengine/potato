// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/find.h"
#include "potato/ecs/component.h"
#include "potato/ecs/query.h"
#include <utility>

namespace up {
    /// An Archetype is a specific configuration of Components.
    ///
    /// All Entities with a given Archetype have the exact same set of
    /// Components.
    class Archetype {
    public:
        UP_ECS_API /*implicit*/ Archetype(std::initializer_list<ComponentId> comps) noexcept;
        UP_ECS_API explicit Archetype(vector<ComponentId> comps) noexcept;

        Archetype(Archetype&&) = delete;
        Archetype& operator=(Archetype&&) = delete;

        bool UP_ECS_API matches(Query const& query) const noexcept;

    private:
        vector<ComponentId> _components;
    };
} // namespace up
