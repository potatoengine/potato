// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/find.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"
#include "potato/foundation/span.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
#include <utility>

namespace up {
    struct EntityChunk;
    class EntityDomain;

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    /// An Archetype is a specific configuration of Components.
    ///
    /// All Entities with a given Archetype have the exact same set of
    /// Components.
    class Archetype : public shared<Archetype> {
    public:
        UP_ECS_API explicit Archetype(EntityDomain& domain, view<ComponentId> comps) noexcept;
        UP_ECS_API ~Archetype();

        Archetype(Archetype&&) = delete;
        Archetype& operator=(Archetype&&) = delete;

        struct Layout {
            ComponentId component = ComponentId::Unknown;
            uint32 offset = 0;
        };

        vector<EntityId> _entities;
        vector<box<EntityChunk>> _chunks;
        vector<Layout> _layout;
        uint32 _count = 0;
        uint32 _perChunk = 0;
        EntityDomain& _domain;
    };
} // namespace up
