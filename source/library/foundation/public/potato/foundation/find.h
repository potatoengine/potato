// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"

namespace up {
    template <class C>
    constexpr auto begin(C& c) noexcept(noexcept(c.begin())) { return c.begin(); }
    template <class C>
    constexpr auto begin(const C& c) noexcept(noexcept(c.begin())) { return c.begin(); }
    template <class T, size_t N>
    constexpr T* begin(T (&array)[N]) noexcept { return array; }

    template <class C>
    constexpr auto end(C& c) noexcept(noexcept(c.end())) { return c.end(); }
    template <class C>
    constexpr auto end(const C& c) noexcept(noexcept(c.end())) { return c.end(); }
    template <class T, size_t N>
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

    template <class T, class P>
    decltype(auto) project(P const& projection, T const& value) noexcept(noexcept(invoke(projection, value))) {
        return invoke(projection, value);
    }
    template <class T, class C>
    decltype(auto) project(T C::*member, T const& value) noexcept {
        return value.*member;
    }

    template <typename C, typename T, typename E = equality, typename P = identity>
    constexpr auto find(C const& container, T const& value, E const& equals = {}, P const& projection = {}) noexcept(noexcept(project(projection, *begin(container)))) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (equals(project(projection, *iter), value)) {
                return iter;
            }
        }

        return last;
    }

    template <typename C, typename P>
    constexpr auto find_if(C const& container, P const& predicate) noexcept(noexcept(predicate(*begin(container)))) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (predicate(*iter)) {
                return iter;
            }
        }

        return last;
    }

    template <typename C, typename P>
    constexpr auto any(C const& container, P const& predicate) noexcept(noexcept(predicate(*begin(container)))) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (predicate(*iter)) {
                return true;
            }
        }

        return false;
    }

    template <typename C, typename P>
    constexpr auto all(C const& container, P const& predicate) noexcept(noexcept(predicate(*begin(container)))) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (!predicate(*iter)) {
                return false;
            }
        }

        return true;
    }

} // namespace up
