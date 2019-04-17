// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/find.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/box.h"
#include "potato/foundation/span.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/ecs/component.h"
#include <utility>

namespace up {
    struct Layout {
        ComponentId component = ComponentId::Unknown;
        uint32 offset = 0;
    };

    struct alignas(32) ChunkHeader {
        int count = 0;
    };
    struct Chunk {
        static constexpr uint32 allocatedSize = 64 * 1024;

        ChunkHeader header;
        char data[allocatedSize - sizeof(header)];
    };
    static_assert(sizeof(Chunk) == Chunk::allocatedSize);

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    /// An Archetype is a specific configuration of Components.
    ///
    /// All Entities with a given Archetype have the exact same set of
    /// Components.
    class Archetype : public shared<Archetype> {
    public:
        UP_ECS_API explicit Archetype(view<ComponentId> comps) noexcept;
        UP_ECS_API ~Archetype();

        Archetype(Archetype&&) = delete;
        Archetype& operator=(Archetype&&) = delete;

        UP_ECS_API bool matches(view<ComponentId> components) const noexcept;
        UP_ECS_API bool matchesExact(view<ComponentId> components) const noexcept;
        UP_ECS_API void unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const;

        UP_ECS_API void* unsafeComponentPointer(uint32 entityIndex, ComponentId component) const noexcept;

        UP_ECS_API uint32 allocateEntity() noexcept;

        UP_ECS_API uint32 unsafeAllocate(view<ComponentId> componentIds, view<void const*> componentData) noexcept;

    private:
        vector<Layout> _layout;
        vector<box<Chunk>> _chunks;
        uint32 _count = 0;
        uint32 _perChunk = 0;
    };
} // namespace up
