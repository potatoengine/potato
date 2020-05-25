// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up {
    template <typename T>
    class sequence {
    public:
        struct sentinel {};

        class iterator {
        public:
            constexpr explicit iterator(T value, T end) noexcept : _value(value), _end(end) {}

            constexpr auto operator*() const noexcept { return _value; }

            constexpr auto operator++() noexcept -> iterator& requires enumeration<T> {
                _value = static_cast<T>(++static_cast<std::underlying_type_t<T>>(_value));
                return *this;
            }

            constexpr auto operator++() noexcept -> iterator& {
                ++_value;
                return *this;
            }

            constexpr auto operator!=(sentinel) const noexcept { return _value != _end; }

        private:
            T _value = {};
            T _end = {};
        };

        using value_type = T;

        constexpr explicit sequence(T end) noexcept : _end(end) {}
        constexpr sequence(T start, T end) noexcept : _start(start), _end(end) {}

        constexpr auto begin() const noexcept -> iterator { return iterator{_start, _end}; }
        constexpr auto end() const noexcept -> sentinel { return {}; }

        constexpr explicit operator bool() const noexcept { return !_start != _end; }

        constexpr bool empty() const noexcept { return _start == _end; }

        constexpr auto size() const noexcept requires enumeration<T> { return static_cast<std::underlying_type_t<T>>(_end) - static_cast<std::underlying_type_t<T>>(_start); }
        constexpr auto size() const noexcept { return _end - _start; }

    private:
        T _start = {};
        T _end = {};
    };

} // namespace up
