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

        using SelectSignature = void(size_t, EntityId const*, Components*...);

        /// Given a World and a callback, finds all matching Archetypes, and invokes the
        /// callback once for each Chunk belonging to the Archetypes, with appropriate pointers.
        ///
        /// This is the primary mechanism for finding or mutating Entities.
        void select(World& world, delegate_ref<SelectSignature> callback);

    private:
        struct MatchedLayout {
            int offsets[sizeof...(Components)];
        };

        template <size_t... Indices>
        void _invoke(std::index_sequence<Indices...>, size_t, EntityId const*, int const* offsets, char* chunkData, delegate_ref<SelectSignature> callback) const;

        uint32 _worldVersion = 0;
        vector<ArchetypeId> _matchedArchetypes;
        vector<MatchedLayout> _matchedLayouts;
    };

    template <typename... Components>
    template <size_t... Indices>
    void Query<Components...>::_invoke(std::index_sequence<Indices...>, size_t count, EntityId const* entities, int const* offsets, char* chunkData, delegate_ref<SelectSignature> callback) const {
        callback(count, entities, static_cast<Components*>(static_cast<void*>(chunkData + offsets[Indices]))...);
    }

    template <typename... Components>
    void Query<Components...>::select(World& world, delegate_ref<SelectSignature> callback) {
        static ComponentId const components[sizeof...(Components)] = {getComponentId<Components>()...};

        if (_worldVersion != world.version()) {
            _worldVersion = world.version();

            _matchedArchetypes.clear();
            _matchedLayouts.clear();

            world.selectArchetypes(components, [this](ArchetypeId arch, view<int> offsets) {
                _matchedArchetypes.push_back(arch);
                _matchedLayouts.emplace_back();

                std::memcpy(&_matchedLayouts.back(), offsets.data(), sizeof(MatchedLayout));
            });
        }

        for (int index = 0; index != _matchedArchetypes.size(); ++index) {
            world.forEachChunk(_matchedArchetypes[index], [this, callback, index](Chunk* chunk) {
                this->_invoke(std::make_index_sequence<sizeof...(Components)>(), chunk->header.count, nullptr, _matchedLayouts[index].offsets, chunk->data, callback);
            });
        }
    }
} // namespace up
