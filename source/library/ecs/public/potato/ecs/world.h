// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/ecs/component.h"
#include "potato/ecs/entity.h"
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

    struct EntityMapping {
        uint32 generation = 0;
        uint32 archetype = 0;
        uint32 index = 0;
    };

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
        EntityId createEntity(Components const&... components) noexcept {
            ComponentId const componentIds[] = {getComponentId<Components>()...};
            void const* componentData[] = {&components...};

            EntityId entity = makeEntityId(static_cast<uint32>(_entities.size()), 0);
            uint32 archetype = _findArchetypeIndex(componentIds);

            uint32 index = _archetypes[archetype]->unsafeAllocate(componentIds, componentData);

            _entities.push_back({getEntityGeneration(entity), archetype, index});

            return entity;
        }

        template <typename Component>
        Component* getComponentSlow(EntityId entity) noexcept {
            return static_cast<Component*>(getComponentSlowUnsafe(entity, getComponentId<Component>()));
        }

        UP_ECS_API void* getComponentSlowUnsafe(EntityId entity, ComponentId component) noexcept;

    private:
        UP_ECS_API uint32 _findArchetypeIndex(view<ComponentId> components) noexcept;

        vector<EntityMapping> _entities;
        vector<rc<Archetype>> _archetypes;
        vector<uint32> _freeList;
    };
} // namespace up
