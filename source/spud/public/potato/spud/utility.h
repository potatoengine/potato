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

    template <typename Value, typename Projection/*, typename = enable_if_t<!std::is_member_object_pointer_v<Projection>>*/>
    decltype(auto) project(Projection const& projection, Value const& value) noexcept(noexcept(invoke(projection, value))) {
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

    template <typename First, typename Last, typename Out, typename Projection = identity>
    constexpr auto copy(First first, Last last, Out out, Projection const& proj = {}) noexcept(noexcept(*out = project(proj, *first))) -> Out {
        for (; first != last; ++first) {
            *out++ = project(proj, *first);
        }
        return out;
    }

    template <typename Enum>
    constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum> {
        return static_cast<std::underlying_type_t<Enum>>(value);
    }

    constexpr auto align_to(std::size_t value, std::size_t alignment) noexcept -> std::size_t {
        auto const alignLessOne = alignment - 1;
        return (value + alignLessOne) & ~alignLessOne;
    }

    template <typename T>
    class sequence {
    public:
        struct sentinel {};

        class iterator {
        public:
            constexpr explicit iterator(T value, T end) noexcept : _value(value), _end(end) {}

            constexpr auto operator*() noexcept { return _value; }

            constexpr auto operator++() noexcept -> iterator& {
                ++_value;
                return *this;
            }

            constexpr auto operator!=(sentinel) const noexcept {
                return _value != _end;
            }

        private:
            T _value = {};
            T _end = {};
        };

        constexpr explicit sequence(T end) noexcept : _end(end) {}
        constexpr explicit sequence(T start, T end) noexcept : _start(start), _end(end) {}

        constexpr auto begin() const noexcept -> iterator { return iterator{_start, _end}; }
        constexpr auto end() const noexcept -> sentinel { return {}; }

    private:
        T _start = {};
        T _end = {};
    };

} // namespace up
