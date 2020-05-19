// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"
#include "functional.h"
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

    template <typename Value, typename Projection, typename = enable_if_t<std::is_invocable_v<Projection, Value const&>>>
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
    constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum> requires std::is_enum_v<Enum> {
        return static_cast<std::underlying_type_t<Enum>>(value);
    }

    template <typename Enum, typename T>
    constexpr auto to_enum(T value) noexcept -> Enum requires std::is_enum_v<Enum>&& std::is_same_v<std::underlying_type_t<Enum>, T> {
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

    namespace _detail {
        template <typename T, bool E = false>
        struct default_sequence_traits {
            static constexpr auto increment(T& value) noexcept -> T& { return ++value; }
            static constexpr auto equal(T start, T end) noexcept -> bool { return start == end; }
            static constexpr auto difference(T start, T end) noexcept -> size_t { return end - start; }
        };

        template <typename T>
        struct default_sequence_traits<T, true> {
            static constexpr auto increment(T& value) noexcept -> T& { return value = static_cast<T>(++to_underlying(value)); }
            static constexpr auto equal(T start, T end) noexcept -> bool { return start == end; }
            static constexpr auto difference(T start, T end) noexcept -> size_t { return to_underlying(end) - to_underlying(start); }
        };
    } // namespace _detail

    template <typename T>
    using sequence_traits = _detail::default_sequence_traits<T, std::is_enum_v<T>>;

    template <typename T, typename Traits = sequence_traits<T>>
    class sequence {
    public:
        struct sentinel {};

        class iterator {
        public:
            constexpr explicit iterator(T value, T end) noexcept : _value(value), _end(end) {}

            constexpr auto operator*() noexcept { return _value; }

            constexpr auto operator++() noexcept -> iterator& {
                Traits::increment(_value);
                return *this;
            }

            constexpr auto operator!=(sentinel) const noexcept { return !Traits::equal(_value, _end); }

        private:
            T _value = {};
            T _end = {};
        };

        constexpr explicit sequence(T end) noexcept : _end(end) {}
        constexpr sequence(T start, T end) noexcept : _start(start), _end(end) {}

        constexpr auto begin() const noexcept -> iterator { return iterator{_start, _end}; }
        constexpr auto end() const noexcept -> sentinel { return {}; }

        constexpr explicit operator bool() const noexcept { return !Traits::equal(_start, _end); }

        constexpr bool empty() const noexcept { return Traits::equal(_start, _end); }

        constexpr auto size() const noexcept { return Traits::difference(_start, _end); }

    private:
        T _start = {};
        T _end = {};
    };

} // namespace up
