// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <initializer_list>
#include <cstddef>

namespace up::_detail {
    template <typename C>
    using has_data_member = enable_if_t<std::is_pointer_v<decltype(std::declval<C>().data())>>;

    template <typename C>
    using has_size_member = enable_if_t<std::is_integral_v<decltype(std::declval<C>().size())>>;

    template <typename C, typename V = void>
    struct has_data : std::false_type {};

    template <typename C>
    struct has_data<C, std::void_t<has_data_member<C>, has_size_member<C>>> : std::true_type {};

    template <typename C>
    constexpr bool has_data_v = has_data<C>::value;
} // namespace up::_detail

namespace up {
    template <typename T>
    struct span;

    template <typename T>
    using view = span<T const>;

    template <typename T>
    span(T*, T*)->span<T>;
    template <typename T>
    span(T*, std::size_t)->span<T>;
    template <typename T, std::size_t N>
    span(T (&src)[N])->span<T>;
    template <typename C, typename = enable_if_t<_detail::has_data_v<C>>>
    span(C &&)->span<std::remove_reference_t<decltype(*std::declval<C>().data())>>;

    template <typename T>
    class vector;

    template <typename T>
    span(vector<T> const&)->span<T const>;
    template <typename T>
    span(vector<T>&)->span<T>;

    template <typename HashAlgorithm, typename T>
    inline void hash_append(HashAlgorithm&, up::span<T> const&) noexcept;
} // namespace up

/// <summary> A non-owning slice of an array. </summary>
/// <typeparam name="T"> Type of the elements in the array. </typeparam>
template <typename T>
struct up::span {
public:
    using value_type = T;
    using iterator = T*;
    using sentinel = T*;
    using pointer = T*;
    using reference = T&;
    using size_type = std::size_t;
    using index_type = size_type;

    constexpr span() noexcept = default;
    constexpr span(span&&) noexcept = default;
    template <typename U>
    /*implicit*/ constexpr span(span<U> src) noexcept
        : _begin(src.begin()), _end(src.end()) {}
    /*implicit*/ constexpr span(T* begin, T* end) noexcept
        : _begin(begin), _end(end) {}
    /*implicit*/ constexpr span(T* ptr, std::size_t size) noexcept
        : _begin(ptr), _end(ptr + size) {}
    template <std::size_t N>
    /*implicit*/ constexpr span(T (&src)[N]) noexcept
        : _begin(src), _end(src + N) {}
    template <typename C, typename = enable_if_t<_detail::has_data_v<C>>>
    /*implicit*/ constexpr span(C&& cont) noexcept : span(cont.data(), cont.size()) {}

    constexpr span& operator=(span&&) noexcept = default;

    constexpr iterator begin() const noexcept { return _begin; }
    constexpr sentinel end() const noexcept { return _end; }

    constexpr pointer data() const noexcept { return _begin; }

    constexpr bool empty() const noexcept { return _begin == _end; }
    constexpr explicit operator bool() const noexcept { return _begin != _end; }

    constexpr reference operator[](size_type index) const noexcept { return _begin[index]; }

    constexpr size_type size() const noexcept { return static_cast<size_type>(_end - _begin); }

    constexpr reference front() const noexcept { return *_begin; }
    constexpr reference back() const noexcept { return *(_end - 1); }

    constexpr span first(size_type length) const noexcept { return span{_begin, length}; };
    constexpr span last(size_type length) const noexcept { return span{_end - length, length}; };

    constexpr span subspan(size_type offset, size_type count) const noexcept { return span{_begin + offset, count}; }

    auto as_bytes() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return span<std::byte const>{reinterpret_cast<std::byte const*>(_begin), static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
        else {
            return span<std::byte>{reinterpret_cast<std::byte*>(_begin), static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
    }

    auto as_chars() const noexcept {
        if constexpr (std::is_const_v<T>) {
            return span<char const>{reinterpret_cast<char const*>(_begin), static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
        else {
            return span<char>{reinterpret_cast<char*>(_begin), static_cast<size_type>(_end - _begin) * sizeof(T)};
        }
    }

    constexpr void pop_front() noexcept { ++_begin; }
    constexpr void pop_back() noexcept { --_end; }

private:
    pointer _begin = nullptr;
    pointer _end = nullptr;
};

template <typename HashAlgorithm, typename T>
void up::hash_append(HashAlgorithm& hasher, up::span<T> const& view) noexcept {
    if constexpr (up::is_contiguous_v<T>) {
        hasher.append_bytes(reinterpret_cast<char const*>(view.data()), view.size() * sizeof(T));
    }
    else {
        for (auto&& value : view) {
            hash_append(hasher, value);
        }
    }
}
