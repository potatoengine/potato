// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"
#include "reflector.h"

#include "potato/spud/zstring_view.h"

#include <typeindex>

namespace up {

    /// @brief Type-erased vtable of operations for a component type
    struct ComponentMetaOps {
        using DefaultConstruct = void (*)(void* dest) noexcept;
        using CopyConstruct = void (*)(void* dest, void const* source) noexcept;
        using MoveAssign = void (*)(void* dest, void* source) noexcept;
        using Destruct = void (*)(void* mem) noexcept;
        using Serialize = void (*)(void* obj, ComponentReflector& reflector);

        DefaultConstruct defaultConstruct = nullptr;
        CopyConstruct copyConstruct = nullptr;
        MoveAssign moveAssign = nullptr;
        Destruct destruct = nullptr;
        Serialize serialize = nullptr;
    };

    /// Stores metadata about a Component type. This includes its size and alignment,
    /// functions for copying and destroying objects of the Component type, and so on.
    ///
    /// Todo: replace with a ubiquitous reflection system of some kind
    struct ComponentMeta {
        zstring_view name;
        ComponentMetaOps ops;
        ComponentId id = ComponentId::Unknown;
        uint64 typeHash = ~0UL;
        uint32 index = ~0U;
        uint32 size = 0;
        uint32 alignment = 0;
    };

    namespace _detail {
        template <typename Component>
        struct ComponentDefaultMetaOps {
            static constexpr void defaultConstruct(void* dest) noexcept { new (dest) Component(); };
            static constexpr void copyConstruct(void* dest, void const* src) noexcept {
                new (dest) Component(*static_cast<Component const*>(src));
            };
            static constexpr void moveAssign(void* dest, void* src) noexcept {
                *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src));
            };
            static constexpr void destruct(void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
            static constexpr void serialize(void* obj, ComponentReflector& reflector) noexcept {
                reflex::serialize(*static_cast<Component*>(obj), reflector);
            };
        };
    } // namespace _detail
} // namespace up
