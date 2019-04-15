// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/find.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/span.h"
#include "potato/foundation/delegate_ref.h"
#include "potato/ecs/component.h"
#include "potato/ecs/query.h"
#include <utility>

namespace up {
    struct Layout {
        ComponentInfo componentInfo;
        uint32 offset = 0;
    };

    using SelectSignature = void(size_t count, view<void*> componentArrays);

    /// An Archetype is a specific configuration of Components.
    ///
    /// All Entities with a given Archetype have the exact same set of
    /// Components.
    class Archetype : public shared<Archetype> {
    public:
        UP_ECS_API explicit Archetype(view<ComponentInfo> comps) noexcept;
        UP_ECS_API ~Archetype();

        Archetype(Archetype&&) = delete;
        Archetype& operator=(Archetype&&) = delete;

        bool UP_ECS_API matches(Query const& query) const noexcept;
        void UP_ECS_API unsafeSelect(Query const& query, delegate_ref<SelectSignature> callback) const noexcept;

        uint32 UP_ECS_API allocateEntity() noexcept;

    private:
        static constexpr uint32 _chunkSize = 64 * 1024;

        vector<Layout> _layout;
        char* _components = nullptr;
        uint32 _count = 0;
        uint32 _perChunk = 0;
    };
} // namespace up
