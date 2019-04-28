// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/typelist.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/delegate.h"
#include "potato/foundation/span.h"
#include "potato/foundation/sort.h"
#include "potato/ecs/component.h"

namespace up {
    /// A Query is used to select a list of Archetypes that provide a particular set of Components,
    /// used to efficiency enumerate all matching Entities.
    template <typename... Components>
    class Query {
    public:
        static_assert(sizeof...(Components) != 0, "Empty Query objects are not allowed");

        using Function = void(size_t, EntityId const*, Components*...);
        using Delegate = delegate_ref<Function>;

        Query() noexcept;

        view<ComponentId> components() const noexcept { return _components; }


        void select(World& world, Delegate callback) const;

    private:
        void _invoke(size_t count, EntityId const* entities, view<void*> pointers, Delegate callback) const;
        template <size_t... Indices>
        void _invokeHelper(std::index_sequence<Indices...>, size_t, EntityId const*, view<void*>, Delegate callback) const;

        ComponentId _components[sizeof...(Components)];
        uint32 _indices[sizeof...(Components)];
    };

    template <typename... Components>
    Query<Components...>::Query() noexcept : _components{} {
        const ComponentId componentIds[] = {getComponentId<Components>()...};

        // Generate a sorted set of indices from the main Components list
        for (uint32 index = 0; index != sizeof...(Components); ++index) {
            _indices[index] = index;
        }

        sort(_indices, {}, [&componentIds](uint32 index) noexcept { return componentIds[index]; });

        // Store the sorted ComponentId list for selection usage
        for (size_t index = 0; index != sizeof...(Components); ++index) {
            _components[index] = componentIds[_indices[index]];
        }
    }

    template <typename... Components>
    template <size_t... Indices>
    void Query<Components...>::_invokeHelper(std::index_sequence<Indices...>, size_t count, EntityId const* entities, view<void*> arrays, Delegate callback) const {
        callback(count, entities, static_cast<Components*>(arrays[_indices[Indices]])...);
    }

    template <typename... Components>
    void Query<Components...>::_invoke(size_t count, EntityId const* entities, view<void*> pointers, Delegate callback) const {
        _invokeHelper(std::make_index_sequence<sizeof...(Components)>(), count, entities, pointers, callback);
    }

    template <typename... Components>
    void Query<Components...>::select(World &world, Delegate callback) const {
        world.selectRaw(_components, [&, this](size_t count, EntityId const* entities, view<void*> arrays) {
            this->_invoke(count, entities, arrays, callback);
        });
    }
} // namespace up
