// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "format_writer.h"

namespace up {
    template <typename ContainerT> class append_writer;
}

/// Writer that calls append(data, size) on wrapped value.
template <typename ContainerT>
class up::append_writer : public up::format_writer {
public:
    constexpr append_writer(ContainerT& container) : _container(container) {}

    constexpr void write(string_view str) override {
        _container.append(str.data(), str.size());
    }

private:
    ContainerT & _container;
};
