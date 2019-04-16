// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/string_view.h"
#include "potato/foundation/nameof.h"
#include "potato/foundation/hash_fnv1a.h"
#include "potato/foundation/traits.h"

namespace up {
    enum class ComponentId : uint64 { Unknown = 0 };

    struct ComponentInfo {
        constexpr ComponentInfo() noexcept : hash(0), size(0), alignment(0) {}
        constexpr ComponentInfo(uint64 h, uint32 s, uint32 a) noexcept : hash(h), size(s), alignment(a) {}

        constexpr ComponentInfo(ComponentId id) noexcept : hash(uint64(id) >> 18), size(uint64(id) >> 5 & 0x1FFF), alignment(uint64(id) & 31) {}

        constexpr operator ComponentId() noexcept { return ComponentId((hash << 18) | (size << 5) | alignment); }

        uint64 hash : 46;
        uint64 size : 13;
        uint64 alignment : 5;
    };
    static_assert(sizeof(ComponentInfo) == sizeof(ComponentId));

    template <typename ComponentT>
    constexpr ComponentId getComponentId() noexcept {
        static_assert(std::is_trivially_copyable_v<ComponentT>, "Component types must be trivially copyable");

        constexpr auto componentName = nameof<ComponentT>();

        fnv1a hasher;
        hasher.append_bytes(componentName.data(), componentName.size());
        uint64 hash = hasher.finalize();

        return ComponentInfo(hash, sizeof(ComponentT), alignof(ComponentT));
    }
} // namespace up
