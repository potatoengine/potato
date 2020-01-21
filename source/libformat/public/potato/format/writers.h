// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#if !defined(_guard_FORMATXX_WRITERS_H)
#define _guard_FORMATXX_WRITERS_H
#pragma once

#include "_detail/format_writer.h"
#include <cstring>

namespace up {
    template <typename ContainerT> class container_writer;
    class span_writer;
}

/// Writer that calls insert(end, range_begin, range_end) on wrapped value.
template <typename ContainerT>
class up::container_writer final : public up::format_writer {
public:
    constexpr container_writer(ContainerT & container) : _container(container) {}

    constexpr void write(string_view str) override {
        _container.insert(_container.end(), str.begin(), str.end());
    }

private:
    ContainerT& _container;
};

/// Writer that appends into a provided memory region, guaranteeing NUL termination and no overflow.
class up::span_writer final : public up::format_writer {
public:
    template <std::size_t Count>
    constexpr span_writer(char(&buffer)[Count]) : _buffer(buffer), _cursor(buffer), _length(Count) {
        *buffer = char{};
    }
    constexpr span_writer(char* buffer, std::size_t length) : _buffer(buffer), _cursor(buffer), _length(length) {
        *buffer = char{};
    }

    void write(string_view str) override {
        std::size_t const size = _cursor - _buffer;
        std::size_t const capacity = _length - 1;
        std::size_t const available = capacity - size;
        std::size_t const length = available < str.size() ? available : str.size();

        std::memcpy(_cursor, str.data(), length);
        _cursor += length;
        *_cursor = char{};
    }

private:
    char* _buffer = nullptr;
    char* _cursor = nullptr;
    std::size_t _length = 0;
};

#endif // !defined(_guard_FORMATXX_WRITERS_H)
