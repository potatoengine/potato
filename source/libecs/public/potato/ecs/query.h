// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "world.h"
#include "archetype.h"
#include "potato/spud/typelist.h"
#include "potato/spud/vector.h"
#include "potato/spud/delegate_ref.h"
#include "potato/spud/span.h"
#include "potato/spud/traits.h"
#include "potato/spud/bit_set.h"
#include "potato/spud/utility.h"
#include "potato/ecs/component.h"

namespace up {
    /// A Query is used to select a list of Archetypes that provide a particular set of Components,
    /// used to efficiency enumerate all matching Entities.
    template <typename... Components>
    class Query {
    public:
        static_assert(sizeof...(Components) != 0, "Empty Query objects are not allowed");

        Query();

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each Chunk belonging to the Archetypes, with appropriate pointers.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback, typename Void = enable_if_t<is_invocable_v<Callback, size_t, Components*...>>>
        void selectChunks(World& world, Callback&& callback);

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each entity.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        ///
        template <typename Callback, typename Void = enable_if_t<is_invocable_v<Callback, Components&...>>>
        void select(World& world, Callback&& callback);

    private:
        struct Match {
            ArchetypeId archetype;
            int offsets[sizeof...(Components)];
        };

        void _refresh(World& world);

        template <typename Callback, size_t... Indices>
        void _executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>) const;
        template <typename Callback, size_t... Indices>
        void _execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const;

        uint32 _worldVersion = 0;
        vector<Match> _matches;
        size_t _matchIndex = 0;
        bit_set _mask;
        ComponentId const _components[sizeof...(Components)] = {};
    };

    template <typename... Components>
    Query<Components...>::Query() : _components{getComponentId<Components>()...} {
        for (auto id : _components) {
            _mask.set(to_underlying(id));
        }
    }

    template <typename... Components>
    template <typename Callback, typename Void>
    void Query<Components...>::selectChunks(World& world, Callback&& callback) {
        _refresh(world);
        _executeChunks(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    template <typename Callback, typename Void>
    void Query<Components...>::select(World& world, Callback&& callback) {
        _refresh(world);
        _execute(world, callback, std::make_index_sequence<sizeof...(Components)>{});
    }

    template <typename... Components>
    void Query<Components...>::_refresh(World& world) {
        auto const currentVersion = world.archetypes().version();
        if (_worldVersion != currentVersion) {
            _worldVersion = currentVersion;

            _matchIndex = world.archetypes().selectArchetypes(_matchIndex, _mask, _components, [this](ArchetypeId arch, view<int> offsets) {
                _matches.emplace_back();
                Match& match = _matches.back();

                match.archetype = arch;
                std::memcpy(&match.offsets, offsets.data(), sizeof(Match::offsets));
            });
        }
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_executeChunks(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.getChunks(match.archetype)) {
                callback(chunk->header.entities, static_cast<Components*>(static_cast<void*>(chunk->data + match.offsets[Indices]))...);
            }
        }
    }

    template <typename... Components>
    template <typename Callback, size_t... Indices>
    void Query<Components...>::_execute(World& world, Callback&& callback, std::index_sequence<Indices...>) const {
        for (auto const& match : _matches) {
            for (auto const& chunk : world.getChunks(match.archetype)) {
                for (unsigned index = 0; index < chunk->header.entities; ++index) {
                    callback(*static_cast<Components*>(static_cast<void*>(chunk->data + match.offsets[Indices]))...);
                }
            }
        }
    }
} // namespace up
