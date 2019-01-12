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
    template <typename T>
    span(T*, std::size_t)->span<T>;

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

    constexpr span() = default;
    template <typename U>
    /*implicit*/ constexpr span(span<U> src) noexcept
        : _begin(src.begin()), _end(src.end()) {}
    template <std::size_t N>
    /*implicit*/ constexpr span(T (&src)[N]) noexcept
        : _begin(src), _end(src + N) {}
    /*implicit*/ constexpr span(T* begin, T* end) noexcept
        : _begin(begin), _end(end) {}
    /*implicit*/ constexpr span(std::initializer_list<T> src) noexcept
        : _begin(src.begin()), _end(src.end()) {}
    template <std::size_t N>
    /*implicit*/ constexpr span(std::array<T, N> src) noexcept : span(src.data(), N) {}
    /*implicit*/ constexpr span(T* ptr, std::size_t size) noexcept
        : _begin(ptr), _end(ptr + size) {}

    constexpr iterator begin() const noexcept { return _begin; }
    constexpr sentinel end() const noexcept { return _end; }

    constexpr pointer data() const noexcept { return _begin; }
    auto data_bytes() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return reinterpret_cast<std::byte const*>(_begin);
        }
        else {
            return reinterpret_cast<std::byte*>(_begin);
        }
    }

    constexpr bool empty() const noexcept { return _begin == _end; }
    constexpr explicit operator bool() const noexcept { return _begin != _end; }

    constexpr reference operator[](size_type index) const noexcept { return _begin[index]; }

    constexpr size_type size() const noexcept { return static_cast<size_type>(_end - _begin); }
    constexpr size_type size_bytes() const noexcept { return sizeof(T) * static_cast<size_type>(_end - _begin); }

    constexpr reference front() const noexcept { return *_begin; }
    constexpr reference back() const noexcept { return *(_end - 1); }

    constexpr span first(size_type length) const noexcept { return span{_begin, length}; };
    constexpr span last(size_type length) const noexcept { return span{_end - length, length}; };

    constexpr span subspan(size_type offset, size_type count) const noexcept { return span{_begin + offset, count}; }

    auto as_bytes() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return span<std::byte const>{reinterpret_cast<std::byte const*>(_begin), static_cast<size_type>(_end - _begin)};
        }
        else {
            return span<std::byte>{reinterpret_cast<std::byte*>(_begin), static_cast<size_type>(_end - _begin)};
        }
    }

    constexpr void pop_front() noexcept { ++_begin; }
    constexpr void pop_back() noexcept { --_end; }

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
