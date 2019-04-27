// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "utility.h"
#include <cstdint>

namespace up {
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
