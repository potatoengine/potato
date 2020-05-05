// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "component.h"

#include <potato/spud/vector.h>
#include <typeindex>

namespace up {
    struct ComponentMeta;

    /// @brief Collects the list of known component types.
    class ComponentRegistry {
    public:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;

        ComponentRegistry(ComponentRegistry const&) = delete;
        ComponentRegistry& operator=(ComponentRegistry const&) = delete;

        UP_ECS_API static auto defaultRegistry() noexcept -> ComponentRegistry&;

        /// @brief Register a component.
        /// @tparam Component Component type to add.
        /// @param name Unique name for this component.
        template <typename Component>
        void registerComponent(zstring_view name);

        /// @brief Deregister a component.
        /// @param id Component to deregister.
        UP_ECS_API void deregisterComponent(ComponentId id);

        /// @brief Locates a component by its name.
        /// @param name Name to lookup by exact match.
        /// @return the found component or nullptr on no match.
        UP_ECS_API auto findByName(string_view name) const noexcept -> ComponentMeta const*;

        /// @brief Locates a component by its unique id.
        /// @param id ID to lookup.
        /// @return the found component or nullptr on no match.
        UP_ECS_API auto findById(ComponentId id) const noexcept -> ComponentMeta const*;

        /// @brief Locates a component by compile-time type.
        /// @tparam Component which type to match against.
        /// @return the found component or nullptr on no match.
        template <typename Component>
        auto findByType() const noexcept -> ComponentMeta const* { return _findByType(typeid(Component).hash_code()); }

        /// @brief Finds the type index for a component by compile-time type.
        /// @tparam Component Component which type to match against
        /// @return the component's id, or UINT_MAX on no match.
        template <typename Component>
        auto findIdByType() const noexcept -> ComponentId;

        /// @brief Finds the type index for a component by compile-time type.
        /// @tparam Component Component which type to match against
        /// @return the component's index, or UINT_MAX on no match.
        template <typename Component>
        auto findIndexByType() const noexcept -> uint32;

        /// @brief Retrieves list of all currently known components.
        /// @return list of known components.
        auto components() const noexcept -> view<ComponentMeta> { return _components; }

    private:
        UP_ECS_API void _registerComponent(ComponentMeta const& meta);
        UP_ECS_API auto _findByType(uint64 typeHash) const noexcept -> ComponentMeta const*;

        vector<ComponentMeta> _components;
        uint32 _nextIndex = 0;
    };

    template <typename Component>
    void ComponentRegistry::registerComponent(zstring_view name) {
        _registerComponent({.name = name,
                            .ops = {
                                .defaultConstruct = _detail::ComponentDefaultMetaOps<Component>::defaultConstruct,
                                .copyConstruct = _detail::ComponentDefaultMetaOps<Component>::copyConstruct,
                                .moveAssign = _detail::ComponentDefaultMetaOps<Component>::moveAssign,
                                .destruct = _detail::ComponentDefaultMetaOps<Component>::destruct,
                                .serialize = _detail::ComponentDefaultMetaOps<Component>::serialize,
                            },
                            .id = to_enum<ComponentId>(hash_value(name)),
                            .typeHash = typeid(Component).hash_code(),
                            .size = sizeof(Component),
                            .alignment = alignof(Component)});
    }

    template <typename Component>
    auto ComponentRegistry::findIdByType() const noexcept -> ComponentId {
        ComponentMeta const* const meta = findByType<Component>();
        return meta != nullptr ? meta->id : ComponentId::Unknown;
    }

    template <typename Component>
    auto ComponentRegistry::findIndexByType() const noexcept -> uint32 {
        ComponentMeta const* const meta = findByType<Component>();
        return meta != nullptr ? meta->index : ~0U;
    }

} // namespace up
