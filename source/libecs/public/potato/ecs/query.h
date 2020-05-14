// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "shared_context.h"
#include "world.h"
#include "component.h"
#include <potato/spud/typelist.h>
#include <potato/spud/vector.h>
#include <potato/spud/span.h>
#include <potato/spud/traits.h>
#include <potato/spud/utility.h>

namespace up {
    /// A Query is used to select a list of Archetypes that provide a particular set of Components,
    /// used to efficiency enumerate all matching Entities.
    template <typename... Components>
    class Query {
    public:
        static_assert(sizeof...(Components) != 0, "Empty Query objects are not allowed");

        explicit Query(EcsSharedContext& context) : _context(context) {}

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each Chunk belonging to the Archetypes, with appropriate pointers.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback, typename Void = enable_if_t<is_invocable_v<Callback, size_t, EntityId const*, Components*...>>>
        void selectChunks(World& world, Callback&& callback);

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each entity.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback, typename Void = enable_if_t<is_invocable_v<Callback, EntityId, Components&...>>>
        void select(World& world, Callback&& callback);

    private:
        using Match = QueryMatch<sizeof...(Components)>;

        template <typename Callback, size_t... Indices>
        void _executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>) const;
        template <typename Callback, size_t... Indices>
        void _execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const;

        vector<Match> _matches;
        size_t _matchIndex = 0;
        EcsSharedContext& _context;
    };

    template <typename... Components>
    template <typename Callback, typename Void>
    void Query<Components...>::selectChunks(World& world, Callback&& callback) {
        _matchIndex = world.matchArchetypesInto<Components...>(_matchIndex, _matches);
        _executeChunks(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <typename Callback, typename Void>
    void Query<Components...>::select(World& world, Callback&& callback) {
        _matchIndex = world.matchArchetypesInto<Components...>(_matchIndex, _matches);
        _execute(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.chunksOf(match.archetype)) {
                callback(chunk->header.entities,
                         static_cast<EntityId const*>(static_cast<void*>(chunk->payload)),
                         static_cast<Components*>(static_cast<void*>(chunk->payload + match.offsets[Indices]))...);
            }
        }
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.chunksOf(match.archetype)) {
                for (unsigned index = 0; index < chunk->header.entities; ++index) {
                    callback(
                        *(static_cast<EntityId*>(static_cast<void*>(chunk->payload)) + index),
                        *(static_cast<Components*>(static_cast<void*>(chunk->payload + match.offsets[Indices])) + index)...);
                }
            }
        }
    }
} // namespace up
