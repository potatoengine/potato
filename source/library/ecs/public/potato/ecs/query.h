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
        using Function = void(size_t, EntityId const*, Components*...);
        using Delegate = delegate<Function>;

        explicit Query(Delegate delegate);

        view<ComponentId> components() const noexcept { return _components; }

        void invokeUnsafe(size_t count, EntityId const* entities, view<void*> pointers);
        void invoke(size_t count, EntityId const* entities, Components*... arrays);

    private:
        template <size_t... Indices>
        void _invokeHelper(std::index_sequence<Indices...>, size_t, EntityId const*, view<void*>);

        vector<ComponentId> _components;
        vector<uint32> _indices;
        Delegate _delegate;
    };

    template <typename... Components>
    Query<Components...>::Query(Delegate delegate) : _components(sizeof...(Components)), _indices(sizeof...(Components)), _delegate(std::move(delegate)) {
        ComponentId componentIds[] = {getComponentId<Components>()..., ComponentId::Unknown/*incase components is empty*/};

        // Generate a sorted set of indices from the main Components list
        for (uint32 index = 0; index != sizeof...(Components); ++index) {
            _indices[index] = index;
        }

        sort(_indices, {}, [&componentIds](uint32 index) noexcept { return componentIds[index]; });

        // Store the sorted ComponentId list for selection usage
        for (size_t index = 0; index != sizeof...(Components); ++index) {
            _components[0] = componentIds[_indices[index]];
        }
    }

    template <typename... Components>
    template <size_t... Indices>
    void Query<Components...>::_invokeHelper(std::index_sequence<Indices...>, size_t count, EntityId const* entities, view<void*> arrays) {
        invoke(count, entities, static_cast<Components*>(arrays[_indices[Indices]])...);
    }

    template <typename... Components>
    void Query<Components...>::invokeUnsafe(size_t count, EntityId const* entities, view<void*> pointers) {
        _invokeHelper(std::make_index_sequence<sizeof...(Components)>(), count, entities, pointers);
    }

    template <typename... Components>
    void Query<Components...>::invoke(size_t count, EntityId const* entities, Components*... arrays) {
        _delegate(count, entities, arrays...);
    }
} // namespace up
