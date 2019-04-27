// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <utility>

namespace up {
    // reimplemented to avoid pulling in the <iterator> monstrosity (bloated in some stdlib implementations)
    template <class C>
    constexpr auto begin(C& c) noexcept(noexcept(c.begin())) { return c.begin(); }
    template <class C>
    constexpr auto begin(const C& c) noexcept(noexcept(c.begin())) { return c.begin(); }
    template <class T, std::size_t N>
    constexpr T* begin(T (&array)[N]) noexcept { return array; }

    template <class C>
    constexpr auto end(C& c) noexcept(noexcept(c.end())) { return c.end(); }
    template <class C>
    constexpr auto end(const C& c) noexcept(noexcept(c.end())) { return c.end(); }
    template <class T, std::size_t N>
    constexpr T* end(T (&array)[N]) noexcept { return array + N; }

    struct identity {
        template <typename T>
        constexpr T const& operator()(T const& value) const noexcept {
            return value;
        }
    };

    struct equality {
        template <typename T, typename U>
        constexpr bool operator()(T const& lhs, U const& rhs) const noexcept(noexcept(lhs == rhs)) {
            return lhs == rhs;
        }
    };

    struct less {
        template <typename T, typename U>
        constexpr bool operator()(T const& lhs, U const& rhs) const noexcept(noexcept(lhs < rhs)) {
            return lhs < rhs;
        }
    };

    template <class T, class P>
    decltype(auto) project(P const& projection, T const& value) noexcept(noexcept(invoke(projection, value))) {
        return invoke(projection, value);
    }
    template <class T, class M>
    M project(M T::*member, T const& value) noexcept {
        return value.*member;
    }
} // namespace up
