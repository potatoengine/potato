// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
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
