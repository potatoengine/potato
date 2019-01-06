// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <initializer_list>
#include <type_traits>
#include <cstddef>
#include <array>

namespace gm {
    template <typename T, typename A>
    class vector;

    template <typename T>
    struct span;

    template <typename T>
    span(std::initializer_list<T>)->span<T const>;
    template <typename T, std::size_t N>
    span(T (&src)[N])->span<T>;

    template <typename HashAlgorithm, typename T>
    inline void hash_append(HashAlgorithm&, gm::span<T> const&) noexcept;
} // namespace gm

/// <summary> A non-owning slice of an array. </summary>
/// <typeparam name="T"> Type of the elements in the array. </typeparam>
template <typename T>
struct gm::span {
public:
    using value_type = T;
    using iterator = T*;
    using sentinel = T*;
    using pointer = T*;
    using reference = T&;
    using size_type = std::size_t;
    using index_type = size_type;

    span() = default; // #FIXME: figure out why this is needed even though I've inherited constructors
    template <typename U>
    /*implicit*/ span(span<U> src) noexcept
        : _begin(src.begin()), _end(src.end()) {}
    template <std::size_t N>
    /*implicit*/ span(T (&src)[N]) noexcept
        : _begin(src), _end(src + N) {}
    /*implicit*/ span(T* begin, T* end) noexcept
        : _begin(begin), _end(end) {}
    /*implicit*/ span(std::initializer_list<T> src) noexcept
        : _begin(src.begin()), _end(src.end()) {}
    template <std::size_t N>
    /*implicit*/ span(std::array<T, N> src) noexcept : span(src.data(), N) {}
    /*implicit*/ span(T* ptr, std::size_t size) noexcept
        : _begin(ptr), _end(ptr + size) {}

    iterator begin() const noexcept { return _begin; }
    sentinel end() const noexcept { return _end; }

    pointer data() const noexcept { return _begin; }

    bool empty() const noexcept { return _begin == _end; }
    explicit operator bool() const noexcept { return _begin != _end; }

    reference operator[](size_type index) const noexcept { return _begin[index]; }

    size_type size() const noexcept { return static_cast<size_type>(_end - _begin); }
    size_type size_bytes() const noexcept { return sizeof(T) * static_cast<size_type>(_end - _begin); }

    reference front() const noexcept { return *_begin; }
    reference back() const noexcept { return *(_end - 1); }

    span first(size_type length) const noexcept { return span{_begin, length}; };
    span last(size_type length) const noexcept { return span{_end - length, length}; };

    span subspan(size_type offset, size_type count) const noexcept { return span{_begin + offset, count}; }

    span<std::byte const> as_bytes() const noexcept {
        return {reinterpret_cast<std::byte const*>(_begin), static_cast<size_type>(_end - _begin)};
    }
    span<std::byte> as_writeable_bytes() const noexcept {
        return {reinterpret_cast<std::byte*>(_begin), _end - _begin};
    }

    void pop_front() noexcept { ++_begin; }
    void pop_back() noexcept { --_end; }

private:
    pointer _begin = nullptr;
    pointer _end = nullptr;
};

template <typename HashAlgorithm, typename T>
void gm::hash_append(HashAlgorithm& hasher, gm::span<T> const& view) noexcept {
    if constexpr (std::is_same_v<std::remove_cv_t<T>, std::byte>) {
        hasher(view);
    }
    else if constexpr (gm::is_contiguous_v<T>) {
        hasher(view.as_bytes());
    }
    else {
        for (auto&& value : view) {
            hash_append(hasher, value);
        }
    }
}
