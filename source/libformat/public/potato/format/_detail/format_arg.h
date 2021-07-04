// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_traits.h"
#include "formatter.h"

#include <initializer_list>
#include <type_traits>

namespace up {
    enum class format_arg_type;

    class format_arg;
    class format_args;

    template <typename Writer, typename T>
    constexpr format_arg make_format_arg(T const& value) noexcept;

    enum class format_arg_type {
        unknown,
        char_t,
        signed_char,
        unsigned_char,
        signed_int,
        unsigned_int,
        signed_short_int,
        unsigned_short_int,
        signed_long_int,
        unsigned_long_int,
        signed_long_long_int,
        unsigned_long_long_int,
        single_float,
        double_float,
        boolean,
        char_string,
        null_pointer,
        void_pointer,
        custom
    };

    /// Abstraction for a single formattable value
    class format_arg {
    public:
        using thunk_type = void (*)(void*, void const*, string_view);

        constexpr format_arg() noexcept = default;
        constexpr format_arg(format_arg_type type, void const* value) noexcept : _type(type), _value(value) {}
        constexpr format_arg(thunk_type thunk, void const* value) noexcept
            : _type(format_arg_type::custom)
            , _thunk(thunk)
            , _value(value) {}

        template <typename Writer>
        constexpr void format_into(Writer& output, string_view spec_string = {}) const;

        constexpr int as_int() const;

    private:
        format_arg_type _type = format_arg_type::unknown;
        thunk_type _thunk = nullptr;
        void const* _value = nullptr;
    };

    /// List of format args.
    ///
    /// Only use this type as a temporary value!
    ///
    class format_args {
    public:
        /*implicit*/ format_args(std::initializer_list<format_arg> args) noexcept
            : args(args.begin())
            , argc(args.size()) {}

        constexpr bool is_valid(int index) const noexcept { return index >= 0 && index < argc; }

        template <typename OutputT>
        constexpr void format_into(OutputT&& output, int index, string_view spec) {
            args[index].format_into(output, spec);
        }

        format_arg const* args = nullptr;
        size_t argc = 0;
    };

    namespace _detail {
        template <typename T>
        struct type_of {
            static constexpr format_arg_type value = format_arg_type::unknown;
        };
#define FORMATXX_TYPE(x, e) \
    template <> \
    struct type_of<x> { \
        static constexpr format_arg_type value = format_arg_type::e; \
    };
        FORMATXX_TYPE(char, char_t);
        FORMATXX_TYPE(signed char, signed_char);
        FORMATXX_TYPE(unsigned char, unsigned_char);
        FORMATXX_TYPE(signed int, signed_int);
        FORMATXX_TYPE(unsigned int, unsigned_int);
        FORMATXX_TYPE(signed short, signed_short_int);
        FORMATXX_TYPE(unsigned short, unsigned_short_int);
        FORMATXX_TYPE(signed long, signed_long_int);
        FORMATXX_TYPE(unsigned long, unsigned_long_int);
        FORMATXX_TYPE(signed long long, signed_long_long_int);
        FORMATXX_TYPE(unsigned long long, unsigned_long_long_int);
        FORMATXX_TYPE(float, single_float);
        FORMATXX_TYPE(double, double_float);
        FORMATXX_TYPE(bool, boolean);
        FORMATXX_TYPE(char*, char_string);
        FORMATXX_TYPE(char const*, char_string);
        FORMATXX_TYPE(std::nullptr_t, null_pointer);
        FORMATXX_TYPE(void*, void_pointer);
        FORMATXX_TYPE(void const*, void_pointer);
#undef FORMTAXX_TYPE

        template <typename OutputT, typename T>
        constexpr void format_value_thunk(void* out, void const* ptr, string_view spec) {
            auto& output = *static_cast<OutputT*>(out);
            auto const& value = *static_cast<T const*>(ptr);
            using formatter_type = up::formatter<T>;
            auto fmt = formatter_type{};
            if constexpr (has_formatter_parse<formatter_type>) {
                char const* const parsed = fmt.parse(spec);
                if (parsed != spec.end()) {
                    return;
                }
            }
            fmt.format(output, value);
        }

        template <typename OutputT, typename T>
        constexpr void format_string_thunk(void* out, void const* ptr, string_view spec) {
            auto& output = *static_cast<OutputT*>(out);
            auto const& value = *static_cast<T const*>(ptr);
            up::formatter<up::string_view> fmt;
            char const* const parsed = fmt.parse(spec);
            if (parsed != spec.end()) {
                return;
            }
            fmt.format(output, value);
        }
    } // namespace _detail

    template <typename OutputT, typename T>
    constexpr format_arg make_format_arg(T const& value) noexcept {
        constexpr format_arg_type type = _detail::type_of<T>::value;

        if constexpr (type != format_arg_type::unknown) {
            return {type, &value};
        }
        else if constexpr (_detail::has_format_value<OutputT, T>) {
            return format_arg(&_detail::format_value_thunk<remove_cvref_t<OutputT>, T>, &value);
        }
        else if constexpr (std::is_convertible_v<T, string_view>) {
            return format_arg(&_detail::format_string_thunk<remove_cvref_t<OutputT>, T>, &value);
        }
        else if constexpr (std::is_pointer_v<T>) {
            return {format_arg_type::void_pointer, &value};
        }
        else if constexpr (std::is_enum_v<T>) {
            return {_detail::type_of<std::underlying_type_t<T>>::value, &value};
        }
        else {
            return {};
        }
    }
} // namespace up
