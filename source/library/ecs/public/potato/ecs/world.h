// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/rc.h"

namespace up {
    class Archetype;
    class Query;

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    /// A world contains a collection of entities, archetypes, and their associated components.
    ///
    /// Entities from different Worlds cannot interact.
    class World {
    public:
        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        void UP_ECS_API unsafeSelect(Query const& query, delegate_ref<SelectSignature> callback) const noexcept;
        rc<Archetype> UP_ECS_API acquireArchetype(view<ComponentId> components) noexcept;

        template <typename... Components>
        void createEntity(Components const&... components) noexcept {
            ComponentId const componentIds[] = {getComponentId<Components>()...};
            void const* componentData[] = {&components...};

            rc<Archetype> archetype = acquireArchetype(componentIds);
            archetype->unsafeAllocate(componentIds, componentData);
        }

    private:
        vector<rc<Archetype>> _archetypes;
    };
} // namespace up
