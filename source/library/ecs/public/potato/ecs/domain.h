// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"
#include "potato/ecs/entity.h"
#include "potato/ecs/archetype.h"

namespace up {
    struct EntityChunk {
        static constexpr uint32 size = 64 * 1024;

        struct alignas(32) Header {
            int count = 0;
        };
        using Payload = char[size - sizeof(Header)];

        Header header;
        Payload data;
    };
    static_assert(sizeof(EntityChunk) == EntityChunk::size);

    struct EntityMapping {
        uint32 generation = 0;
        uint32 archetype = 0;
        uint32 index = 0;
    };

    class EntityDomain {
    public:
        EntityDomain() = default;
        UP_ECS_API ~EntityDomain();

        EntityDomain(EntityDomain const&) = delete;
        EntityDomain& operator=(EntityDomain const&) = delete;

        UP_ECS_API EntityId allocateEntityId() noexcept;
        UP_ECS_API void returnEntityId(EntityId entity) noexcept;

        UP_ECS_API box<EntityChunk> allocateChunk();
        UP_ECS_API void returnChunk(box<EntityChunk> chunk) noexcept;

        vector<EntityMapping> entityMapping;
        vector<rc<Archetype>> archetypes;

    private:
        vector<box<EntityChunk>> _chunkPool;
        uint32 _freeEntityHead = static_cast<uint32>(-1);
    };
} // namespace up
