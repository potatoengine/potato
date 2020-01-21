// up - C++ string formatting library.
//
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non - commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//
// Authors:
//   Sean Middleditch <sean@middleditch.us>

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
