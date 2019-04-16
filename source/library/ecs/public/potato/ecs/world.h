// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/foundation/rc.h"

namespace up {
    class Archetype;

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    namespace _detail {
        template <size_t... Indices, typename... Components>
        void innerSelectHelper(std::index_sequence<Indices...>, size_t count, view<void*> arrays, delegate_ref<void(size_t count, Components*... components)> callback) {
            callback(count, static_cast<Components*>(arrays[Indices])...);
        }

        template <typename... Components>
        void selectHelper(size_t count, view<void*> arrays, delegate_ref<void(size_t count, Components*... components)> callback) {
            innerSelectHelper(std::make_index_sequence<sizeof...(Components)>(), count, arrays, callback);
        }

    } // namespace _detail

    /// A world contains a collection of entities, archetypes, and their associated components.
    ///
    /// Entities from different Worlds cannot interact.
    class World {
    public:
        UP_ECS_API World();
        UP_ECS_API ~World();

        World(World&&) = delete;
        World& operator=(World&&) = delete;

        void UP_ECS_API unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const;

        template <typename... Components>
        void select(delegate_ref<void(size_t count, Components*... components)> callback) const {
            unsafeSelect(view<ComponentId>({getComponentId<Components>()...}), [&callback](size_t count, view<void*> arrays) {
                _detail::selectHelper<Components...>(count, arrays, callback);
            });
        }

        rc<Archetype> UP_ECS_API acquireArchetype(view<ComponentId> components) noexcept;

        view<rc<Archetype>> archetypes() const noexcept { return _archetypes; }

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
