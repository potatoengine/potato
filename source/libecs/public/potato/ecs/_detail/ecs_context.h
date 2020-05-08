// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "../component.h"
#include "../chunk.h"
#include <potato/spud/int_types.h>
#include <potato/spud/span.h>
#include <potato/spud/vector.h>
#include <potato/spud/find.h>

namespace up::_detail {
    struct EcsContext {
        inline auto findById(ComponentId id) const noexcept -> ComponentMeta const*;
        inline auto findByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const*;

        template <typename Component>
        auto findByType() const noexcept -> ComponentMeta const* { return findByTypeHash(typeid(Component).hash_code()); }

        inline auto allocate(ArchetypeId archetype) -> Chunk*;
        inline void recycle(Chunk* chunk) noexcept;

        vector<ComponentMeta> components;
        vector<box<Chunk>> _chunks;
        Chunk* _freeChunkHead = nullptr;
    };

    auto EcsContext::findById(ComponentId id) const noexcept -> ComponentMeta const* {
        auto const* it = find(components, id, equality{}, &ComponentMeta::id);
        return it != components.end() ? it : nullptr;
    }

    auto EcsContext::findByTypeHash(uint64 typeHash) const noexcept -> ComponentMeta const* {
        auto const* it = find(components, typeHash, equality{}, &ComponentMeta::typeHash);
        return it != components.end() ? it : nullptr;
    }

    auto EcsContext::allocate(ArchetypeId archetype) -> Chunk* {
        if (_freeChunkHead != nullptr) {
            Chunk* const chunk = _freeChunkHead;
            _freeChunkHead = chunk->header.next;
            chunk->header.archetype = archetype;
            chunk->header.next = nullptr;
            return chunk;
        }

        _chunks.push_back(new_box<Chunk>());
        Chunk* const chunk = _chunks.back().get();
        chunk->header.archetype = archetype;
        return chunk;
    }

    void EcsContext::recycle(Chunk* chunk) noexcept {
        if (chunk == nullptr) {
            return;
        }

        chunk->header.archetype = ArchetypeId::Empty;
        chunk->header.entities = 0;
        chunk->header.next = _freeChunkHead;
        _freeChunkHead = chunk;
    }
} // namespace up::_detail
