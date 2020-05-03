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

    /// Stores metadata about a Component type. This includes its size and alignment,
    /// functions for copying and destroying objects of the Component type, and so on.
    ///
    /// Todo: replace with a ubiquitous reflection system of some kind
    struct ComponentMeta {
        using Construct = void (*)(void* dest) noexcept;
        using Copy = void (*)(void* dest, void const* source) noexcept;
        using Relocate = void (*)(void* dest, void* source) noexcept;
        using Destroy = void (*)(void* mem) noexcept;
        using Reflect = void (*)(void* obj, ComponentReflector& reflector);

        /// Creates a ComponentMeta; should only be used by the UP_COMPONENT macro
        ///
        template <typename Component>
        static constexpr auto createMeta(zstring_view name) noexcept -> ComponentMeta;

        ComponentId id = ComponentId::Unknown;
        std::type_index type = typeid(void);
        uint32 index = ~0U;
        Construct construct = nullptr;
        Copy copy = nullptr;
        Relocate relocate = nullptr;
        Destroy destroy = nullptr;
        Reflect reflect = nullptr;
        uint32 size = 0;
        uint32 alignment = 0;
        zstring_view name;
    };

    namespace _detail {
        template <typename Component>
        struct ComponentOperations {
            static constexpr void constructComponent(void* dest) noexcept { new (dest) Component(); };
            static constexpr void copyComponent(void* dest, void const* src) noexcept { new (dest) Component(*static_cast<Component const*>(src)); };
            static constexpr void moveComponent(void* dest, void* src) noexcept { *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src)); };
            static constexpr void destroyComponent(void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
            static constexpr void reflectComponent(void* obj, ComponentReflector& reflector) noexcept { reflex::serialize(*static_cast<Component*>(obj), reflector); };
        };
    } // namespace _detail

    template <typename Component>
    constexpr ComponentMeta ComponentMeta::createMeta(zstring_view name) noexcept {
        ComponentMeta meta;
        meta.id = to_enum<ComponentId>(hash_value(name));
        meta.type = typeid(Component);
        meta.construct = _detail::ComponentOperations<Component>::constructComponent;
        meta.copy = _detail::ComponentOperations<Component>::copyComponent;
        meta.relocate = _detail::ComponentOperations<Component>::moveComponent;
        meta.destroy = _detail::ComponentOperations<Component>::destroyComponent;
        meta.reflect = _detail::ComponentOperations<Component>::reflectComponent;
        meta.size = sizeof(Component);
        meta.alignment = alignof(Component);
        meta.name = name;
        return meta;
    }
} // namespace up
