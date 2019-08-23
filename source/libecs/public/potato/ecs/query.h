// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "world.h"
#include "potato/spud/typelist.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/span.h"
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

        /// Constructs a new Query object.
        ///
        /// This is a non-trivial operation and Query objects should be cached and reused.
        Query() noexcept;

        /// Fetches the sorted list of ComponentIds required by this Query.
        view<ComponentId> components() const noexcept { return _components; }

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each Chunk belonging to the Archetypes, with appropriate pointers.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        void select(World& world, Delegate callback) const;

    private:
        struct MatchedLayout {
            int offsets[sizeof...(Components)];
        };

        template <size_t... Indices>
        void _invoke(std::index_sequence<Indices...>, size_t, EntityId const*, view<void*>, Delegate& callback) const;

        void _match(World& world) const;

        mutable vector<ArchetypeId> _matchedArchetypes;
        mutable vector<MatchedLayout> _matchedLayouts;
        ComponentId _components[sizeof...(Components)];
    };

    template <typename... Components>
    Query<Components...>::Query() noexcept : _components{getComponentId<Components>()...} {}

    template <typename... Components>
    template <size_t... Indices>
    void Query<Components...>::_invoke(std::index_sequence<Indices...>, size_t count, EntityId const* entities, view<void*> arrays, Delegate& callback) const {
        callback(count, entities, static_cast<Components*>(arrays[Indices])...);
    }

    template <typename... Components>
    void Query<Components...>::select(World &world, Delegate callback) const {
        _match(world);

        for (ArchetypeId id : _matchedArchetypes) {
            world.selectRaw(id, _components, [&, this](size_t count, EntityId const* entities, view<void*> arrays) {
                this->_invoke(std::make_index_sequence<sizeof...(Components)>(), count, entities, arrays, callback);
            });
        }
    }

    template <typename... Components>
    void Query<Components...>::_match(World& world) const {
        _matchedArchetypes.clear();
        world.selectArchetypes(_components, _matchedArchetypes);
    }
} // namespace up
