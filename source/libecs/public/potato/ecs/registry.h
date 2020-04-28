// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

#include <potato/spud/vector.h>

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
        /// @param meta A component metadata object, which cannot have been previously registered.
        UP_ECS_API void registerComponent(ComponentMeta const* meta);

        /// @brief Deregister a component.
        /// @param meta A component metadata object, which must have been previously registered.
        UP_ECS_API void deregisterComponent(ComponentMeta const* meta);

        /// @brief Locates a component by its name.
        /// @param name Name to lookup by exact match.
        /// @return the found component or nullptr on no match.
        UP_ECS_API auto findByName(string_view name) const noexcept -> ComponentMeta const*;

        /// @brief Retrieves list of all currently known components.
        /// @return list of known components.
        auto components() const noexcept -> view<ComponentMeta const*> { return _components; }

    private:
        vector<ComponentMeta const*> _components;
    };
} // namespace up
