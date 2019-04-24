// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/ecs/common.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/nameof.h"
#include "potato/foundation/hash_fnv1a.h"
#include "potato/foundation/traits.h"
#include <utility>
#include <cstring>

namespace up {
    struct ComponentMeta {
        using Copy = void (*)(void* dest, void const* source) noexcept;
        using Relocate = void (*)(void* dest, void* source) noexcept;
        using Destroy = void (*)(void* mem) noexcept;

        template <typename Component>
        static constexpr ComponentMeta construct() noexcept;

        template <typename Component>
        static constexpr ComponentMeta const* get() noexcept;

        ComponentId id = ComponentId::Unknown;
        Copy copy = nullptr;
        Relocate relocate = nullptr;
        Destroy destroy = nullptr;
        uint32 size = 0;
        uint32 alignment = 0;
        string_view name;

        template <typename Component>
        struct holder {
            static ComponentMeta const meta;
        };
    };

    namespace _detail {
        constexpr uint64 hashComponentName(string_view name) noexcept {
            fnv1a hasher;
            hasher.append_bytes(name.data(), name.size());
            return hasher.finalize();
        }

        template <uint64 hash>
        constexpr ComponentId componentIdFromHash = static_cast<ComponentId>(hash);
    }

    template <typename Component>
    constexpr ComponentMeta ComponentMeta::construct() noexcept {
        constexpr string_view name = nameof<Component>();

        fnv1a hasher;
        hasher.append_bytes(name.data(), name.size());
        uint64 hash = hasher.finalize();

        ComponentMeta meta;
        meta.id = _detail::componentIdFromHash<_detail::hashComponentName(name)>;
        meta.copy = [](void* dest, void const* src) noexcept { *static_cast<Component*>(dest) = *static_cast<Component const*>(src); };
        meta.relocate = [](void* dest, void* src) noexcept { *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src)); };
        meta.destroy = [](void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
        meta.size = sizeof(Component);
        meta.alignment = alignof(Component);
        meta.name = name;
        return meta;
    }

    template <typename Component>
    constexpr ComponentMeta const* ComponentMeta::get() noexcept {
        return &holder<Component>::meta;
    }

    #define UP_COMPONENT(ComponentType) \
        template <> \
        up::ComponentMeta const up::ComponentMeta::holder<ComponentType>::meta = up::ComponentMeta::construct<ComponentType>();

    template <typename ComponentT>
    constexpr ComponentId getComponentId() noexcept {
        return _detail::componentIdFromHash<_detail::hashComponentName(nameof<ComponentT>())>;
    }
} // namespace up
