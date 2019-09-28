// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/traits.h"

namespace up {
    /// Stores metadata about a Component type. This includes its size and alignment,
    /// functions for copying and destroying objects of the Component type, and so on.
    ///
    /// Todo: replace with a ubiquitous reflection system of some kind
    struct ComponentMeta {
        using Copy = void (*)(void* dest, void const* source) noexcept;
        using Relocate = void (*)(void* dest, void* source) noexcept;
        using Destroy = void (*)(void* mem) noexcept;

        /// Creates a ComponentMeta; should only be used by the UP_COMPONENT macro
        template <typename Component>
        static constexpr auto construct(zstring_view name) noexcept -> ComponentMeta;

        /// Retrieves the ComponentMeta for a given type
        template <typename Component>
        static constexpr auto get() noexcept -> ComponentMeta const*;

        ComponentId id = ComponentId::Unknown;
        Copy copy = nullptr;
        Relocate relocate = nullptr;
        Destroy destroy = nullptr;
        uint32 size = 0;
        uint32 alignment = 0;
        zstring_view name;
    };

    namespace _detail {
        constexpr auto hashComponentName(zstring_view name) noexcept -> uint64 {
            fnv1a hasher;
            hasher.append_bytes(name.data(), name.size());
            return hasher.finalize();
        }

        template <uint64 hash>
        constexpr ComponentId componentIdFromHash = static_cast<ComponentId>(hash);

        template <typename Component>
        struct ComponentOperations {
            static constexpr void copyComponent(void* dest, void const* src) noexcept { new(dest) Component(*static_cast<Component const*>(src)); };
            static constexpr void moveComponent(void* dest, void* src) noexcept { *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src)); };
            static constexpr void destroyComponent(void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
        };

        template <typename Component>
        struct MetaHolder;
    }

    template <typename Component>
    constexpr ComponentMeta ComponentMeta::construct(zstring_view name) noexcept {
        fnv1a hasher;
        hasher.append_bytes(name.data(), name.size());
        uint64 hash = hasher.finalize();

        ComponentMeta meta;
        meta.id = static_cast<ComponentId>(hash);
        meta.copy = _detail::ComponentOperations<Component>::copyComponent;
        meta.relocate = _detail::ComponentOperations<Component>::moveComponent;
        meta.destroy = _detail::ComponentOperations<Component>::destroyComponent;
        meta.size = sizeof(Component);
        meta.alignment = alignof(Component);
        meta.name = name;
        return meta;
    }

    template <typename ComponentT>
    constexpr auto ComponentMeta::get() noexcept -> ComponentMeta const* {
        return &_detail::MetaHolder<litexx::remove_cvref_t<ComponentT>>::meta;
    }

    /// Registers a type as a Component and creates an associated ComponentMeta
    #define UP_COMPONENT(ComponentType, ...) \
        template <> \
        struct up::_detail::MetaHolder<ComponentType> { \
            __VA_ARGS__ static constexpr up::ComponentMeta meta = up::ComponentMeta::construct<ComponentType>(#ComponentType); \
        };

    /// Finds the unique ComponentId for a given Component type
    template <typename ComponentT>
    constexpr auto getComponentId() noexcept -> ComponentId {
        return _detail::MetaHolder<litexx::remove_cvref_t<ComponentT>>::meta.id;
    }
} // namespace up
