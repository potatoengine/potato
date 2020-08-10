// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "functional.h"
#include "traits.h"

#include <utility>

namespace up {
    // reimplemented to avoid pulling in the <iterator> monstrosity (bloated in some stdlib implementations)
    template <class C>
    constexpr auto begin(C&& c) noexcept(noexcept(c.begin())) {
        return c.begin();
    }
    template <class T, std::size_t N>
    constexpr T* begin(T (&array)[N]) noexcept {
        return array;
    }

    template <class C>
    constexpr auto end(C&& c) noexcept(noexcept(c.end())) {
        return c.end();
    }
    template <class T, std::size_t N>
    constexpr T* end(T (&array)[N]) noexcept {
        return array + N;
    }

    struct identity {
        template <typename T>
        constexpr T const& operator()(T const& value) const noexcept {
            return value;
        }
    };

    struct equality {
        template <typename T, equality_comparable_with<T> U>
        constexpr bool operator()(T const& lhs, U const& rhs) const noexcept(noexcept(lhs == rhs)) {
            return lhs == rhs;
        }
    };

    struct less {
        template <typename T, typename U>
        requires less_than_comparable_with<T, U> constexpr bool operator()(T const& lhs, U const& rhs) const
            noexcept(noexcept(lhs < rhs)) {
            // appears to be a clang-tidy bug?
            //
            // NOLINTNEXTLINE(modernize-use-nullptr)
            return lhs < rhs;
        }
    };

    template <typename Value, callable<Value const&> Projection>
    decltype(auto) project(Projection const& projection, Value const& value) noexcept(
        noexcept(invoke(projection, value))) {
        return invoke(projection, value);
    }
    template <typename Class, typename ReturnType>
    auto project(ReturnType Class::*member, Class const& value) noexcept -> ReturnType {
        return value.*member;
    }
    template <typename Class, typename ReturnType>
    auto project(ReturnType Class::*member, Class const* value) noexcept -> ReturnType {
        return value->*member;
    }

    template <typename First, typename Last, typename Out, projection<deref_t<First> const&> Projection>
    constexpr auto copy(First first, Last last, Out out, Projection const& proj = {}) noexcept(
        noexcept(*out = project(proj, *first))) -> Out {
        for (; first != last; ++first) {
            *out++ = project(proj, *first);
        }
        return out;
    }

    template <typename First, typename Last, typename Out>
    constexpr auto copy(First first, Last last, Out out) noexcept(noexcept(*out = *first)) -> Out {
        for (; first != last; ++first) {
            *out++ = *first;
        }
        return out;
    }

    template <enumeration Enum>
    constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum> {
        return static_cast<std::underlying_type_t<Enum>>(value);
    }

    template <enumeration Enum, same_as<std::underlying_type_t<Enum>> T>
    constexpr auto to_enum(T value) noexcept -> Enum {
        return static_cast<Enum>(value);
    }

    constexpr auto align_to(std::size_t value, std::size_t alignment) noexcept -> std::size_t {
        auto const alignLessOne = alignment - 1;
        return (value + alignLessOne) & ~alignLessOne;
    }

    template <typename To, typename From>
    constexpr auto narrow_cast(From&& from) noexcept -> To {
        return static_cast<To>(std::forward<From>(from));
    }

    template <typename T>
    struct tag {};

} // namespace up
