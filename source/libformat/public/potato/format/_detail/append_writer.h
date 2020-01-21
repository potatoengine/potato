// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#if !defined(_guard_FORMATXX_DETAIL_APPEND_WRITER_H)
#define _guard_FORMATXX_DETAIL_APPEND_WRITER_H
#pragma once

#include <potato/spud/string_view.h>
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

#endif // !defined(_guard_FORMATXX_DETAIL_APPEND_WRITER_H)
