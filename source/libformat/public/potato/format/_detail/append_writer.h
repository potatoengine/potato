// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include <type_traits>

namespace up {
    /// Writer that calls append(data, size) on wrapped value.
    template <typename ContainerT>
    class append_writer {
    public:
        constexpr append_writer(ContainerT& container) noexcept : _container(container) {}

        constexpr void write(string_view str) noexcept(noexcept(std::declval<ContainerT>().append(str.data(), str.size()))) {
            _container.append(str.data(), str.size());
        }

    private:
        ContainerT& _container;
    };
} // namespace up
