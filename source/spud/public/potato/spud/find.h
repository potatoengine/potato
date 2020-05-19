// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "utility.h"
#include <cstdint>

namespace up {
    template <typename C, typename T>
    constexpr auto find(C const& container, T const& value) noexcept(noexcept(*begin(container) == value)) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (*iter == value) {
                return iter;
            }
        }

        return last;
    }

    template <typename C, typename T, typename E = equality, typename P = identity>
    constexpr auto find(C const& container, T const& value, E const& equals, P const& projection = {}) noexcept(noexcept(equals(project(projection, *begin(container)), value))) {
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

    template <typename C, typename T>
    constexpr auto contains(C const& container, T const& value) noexcept(noexcept(*begin(container) == value)) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (*iter == value) {
                return true;
            }
        }

        return false;
    }

    template <typename C, typename T, typename E = equality, typename P = identity>
    constexpr auto contains(C const& container, T const& value, E const& equals, P const& projection = {}) noexcept(noexcept(equals(project(projection, *begin(container)), value))) {
        auto iter = begin(container);
        auto last = end(container);

        for (; iter != last; ++iter) {
            if (equals(project(projection, *iter), value)) {
                return true;
            }
        }

        return false;
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
