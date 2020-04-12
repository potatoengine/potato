// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/traits.h"
#include "potato/reflex/reflect.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <atomic>
#include <new>

namespace up {
    class ComponentReflector {
    public:
        virtual ~ComponentReflector() = default;

        template <typename ObjectType, typename ClassType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field, Annotations&& annotations) {
            onField(name);
            value(object.*field);
        }

        template <typename ObjectType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType&& field, Annotations&& annotations) {
            onField(name);
            value(field);
        }

        template <typename ValueType>
        void value(ValueType&& value) {
            onValue(value);
        }

    protected:
        virtual void onField(zstring_view name) = 0;
        virtual void onValue(EntityId value) = 0;
        virtual void onValue(int& value) = 0;
        virtual void onValue(float& value) = 0;
        virtual void onValue(glm::vec3& value) = 0;
        virtual void onValue(glm::quat& value) = 0;
    };

    /// Stores metadata about a Component type. This includes its size and alignment,
    /// functions for copying and destroying objects of the Component type, and so on.
    ///
    /// Todo: replace with a ubiquitous reflection system of some kind
    struct ComponentMeta {
        using Copy = void (*)(void* dest, void const* source) noexcept;
        using Relocate = void (*)(void* dest, void* source) noexcept;
        using Destroy = void (*)(void* mem) noexcept;
        using Reflect = void (*)(void* obj, ComponentReflector& reflector);

        /// Creates a ComponentMeta; should only be used by the UP_COMPONENT macro
        ///
        template <typename Component>
        static constexpr auto construct(zstring_view name) noexcept -> ComponentMeta;

        /// Retrieves the ComponentMeta for a given type
        ///
        template <typename Component>
        static constexpr auto get() noexcept -> ComponentMeta const&;

        /// Assigns a unique system-wide ID to the component.
        ///
        UP_ECS_API static auto allocateId() noexcept -> ComponentId;

        ComponentId id = ComponentId::Unknown;
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
            static constexpr void copyComponent(void* dest, void const* src) noexcept { new (dest) Component(*static_cast<Component const*>(src)); };
            static constexpr void moveComponent(void* dest, void* src) noexcept { *static_cast<Component*>(dest) = std::move(*static_cast<Component*>(src)); };
            static constexpr void destroyComponent(void* mem) noexcept { static_cast<Component*>(mem)->~Component(); };
            static constexpr void reflectComponent(void* obj, ComponentReflector& reflector) noexcept { reflex::serialize(*static_cast<Component*>(obj), reflector); };
        };

        template <typename Component>
        struct MetaHolder;
    } // namespace _detail

    template <typename Component>
    constexpr ComponentMeta ComponentMeta::construct(zstring_view name) noexcept {
        ComponentMeta meta;
        meta.id = allocateId();
        meta.copy = _detail::ComponentOperations<Component>::copyComponent;
        meta.relocate = _detail::ComponentOperations<Component>::moveComponent;
        meta.destroy = _detail::ComponentOperations<Component>::destroyComponent;
        meta.reflect = _detail::ComponentOperations<Component>::reflectComponent;
        meta.size = sizeof(Component);
        meta.alignment = alignof(Component);
        meta.name = name;
        return meta;
    }

    /// Finds the unique ComponentId for a given Component type
    ///
    template <typename ComponentT>
    constexpr auto getComponentId() noexcept -> ComponentId {
        return ComponentMeta::get<ComponentT>().id;
    }

    template <typename ComponentT>
    constexpr auto ComponentMeta::get() noexcept -> ComponentMeta const& {
        return _detail::MetaHolder<remove_cvref_t<ComponentT>>::get();
    }

/// Declare a type is a Component and can be accessed via ComponentMeta::get()
///
#define UP_DECLARE_COMPONENT(ComponentType, ...) \
    template <> \
    struct up::_detail::MetaHolder<ComponentType> { \
        __VA_ARGS__ static auto get() noexcept -> ComponentMeta const&; \
    };

/// Registers a type with the component manager
///
#define UP_DEFINE_COMPONENT(ComponentType) \
    auto up::_detail::MetaHolder<ComponentType>::get() noexcept->ComponentMeta const& { \
        static auto const meta = ::up::ComponentMeta::construct<ComponentType>(#ComponentType); \
        return meta; \
    }

} // namespace up
