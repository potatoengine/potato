// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "reflector.h"
#include <potato/spud/zstring_view.h>
#include <potato/spud/traits.h>
#include <potato/spud/hash.h>
#include <typeindex>
#include <atomic>
#include <new>

namespace up {

    /// @brief Type-erased vtable of operations for a component type
    struct ComponentOps {
        using DefaultConstruct = void (*)(void* dest) noexcept;
        using CopyConstruct = void (*)(void* dest, void const* source) noexcept;
        using MoveAssign = void (*)(void* dest, void* source) noexcept;
        using Destruct = void (*)(void* mem) noexcept;
        using Reflect = void (*)(void* obj, ComponentReflector& reflector);

        DefaultConstruct defaultConstruct = nullptr;
        CopyConstruct copyConstruct = nullptr;
        MoveAssign moveAssign = nullptr;
        Destruct destruct = nullptr;
        Reflect reflect = nullptr;

        template <typename Component>
        static constexpr auto createOps() noexcept -> ComponentOps;
    };

    /// Stores metadata about a Component type. This includes its size and alignment,
    /// functions for copying and destroying objects of the Component type, and so on.
    ///
    /// Todo: replace with a ubiquitous reflection system of some kind
    struct ComponentMeta {

        /// Creates a ComponentMeta; should only be used by the UP_COMPONENT macro
        ///
        template <typename Component>
        static auto createMeta(zstring_view name) noexcept -> ComponentMeta;

        zstring_view name;
        ComponentOps ops;
        ComponentId id = ComponentId::Unknown;
        uint64 typeHash = ~0UL;
        uint32 index = ~0U;
        uint32 size = 0;
        uint32 alignment = 0;
    };

    namespace _detail {
        template <typename Component>
        struct ComponentDefaultOps {
            static constexpr void defaultConstruct(void* dest) noexcept { new (dest) Component(); };
            static constexpr void copyConstruct(void* dest, void const* src) noexcept { new (dest) Component(*static_cast<Component const*>(src)); };
            static constexpr void moveAssign(void* dest, void* src) noexcept { *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src)); };
            static constexpr void destruct(void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
            static constexpr void reflect(void* obj, ComponentReflector& reflector) noexcept { reflex::serialize(*static_cast<Component*>(obj), reflector); };
        };
    } // namespace _detail

    template <typename Component>
    constexpr auto ComponentOps::createOps() noexcept -> ComponentOps {
        return {
            .defaultConstruct = _detail::ComponentDefaultOps<Component>::defaultConstruct,
            .copyConstruct = _detail::ComponentDefaultOps<Component>::copyConstruct,
            .moveAssign = _detail::ComponentDefaultOps<Component>::moveAssign,
            .destruct = _detail::ComponentDefaultOps<Component>::destruct,
            .reflect = _detail::ComponentDefaultOps<Component>::reflect,
        };
    }

    template <typename Component>
    auto ComponentMeta::createMeta(zstring_view name) noexcept -> ComponentMeta {
        return {
            .name = name,
            .ops = ComponentOps::createOps<Component>(),
            .id = to_enum<ComponentId>(hash_value(name)),
            .typeHash = typeid(Component).hash_code(),
            .size = sizeof(Component),
            .alignment = alignof(Component)};
    }
} // namespace up
