// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/string_view.h"
#include "potato/foundation/nameof.h"
#include "potato/foundation/hash_fnv1a.h"

namespace up {
    enum class ComponentId : uint32 { Unknown = 0 };

    template <typename ComponentT>
    constexpr ComponentId getComponentId() noexcept {
        
        constexpr auto componentName = nameof<ComponentT>();

        fnv1a hasher;
        hasher.append_bytes(componentName.data(), componentName.size());
        return static_cast<ComponentId>(hasher.finalize());
    }
} // namespace up
