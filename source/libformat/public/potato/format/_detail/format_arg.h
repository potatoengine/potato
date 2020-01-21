// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "format_options.h"

namespace up::_detail {
    enum class format_arg_type;

    class format_arg;
    class format_arg_list;

    template <typename T> constexpr format_arg make_format_arg(T const& value) noexcept;
}

enum class up::_detail::format_arg_type {
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
class up::_detail::format_arg {
public:
    using thunk_type = result_code(*)(format_writer&, void const*, format_options);

    constexpr format_arg() noexcept = default;
    constexpr format_arg(_detail::format_arg_type type, void const* value) noexcept : _type(type), _value(value) {}
    constexpr format_arg(thunk_type thunk, void const* value) noexcept : _type(_detail::format_arg_type::custom), _thunk(thunk), _value(value) {}

    UP_FORMAT_API result_code format_into(format_writer& output, format_options options) const;

private:
    _detail::format_arg_type _type = _detail::format_arg_type::unknown;
    thunk_type _thunk = nullptr;
    void const* _value = nullptr;
};

/// Abstraction for a set of format arguments.
class up::_detail::format_arg_list {
public:
    using format_arg_type = format_arg;
    using thunk_type = typename format_arg_type::thunk_type;
    using size_type = std::size_t;

    constexpr format_arg_list() noexcept = default;
    constexpr format_arg_list(format_arg_type const* args, size_type count) noexcept : _args(args), _count(count) {}

    constexpr result_code format_arg_into(format_writer& output, size_type index, format_options options) const {
        return index < _count ? _args[index].format_into(output, options) : result_code::out_of_range;
    }

private:
    format_arg const* _args = nullptr;
    size_type _count = 0;
};

namespace up::_detail {
    template <typename T>
    using decay_array_t = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T> const*, T>;

    template <typename T>
    using formattable_t = decay_array_t<std::remove_reference_t<T>>;

    template <typename T, typename V = void>
    struct has_format_value { static constexpr bool value = false; };
    template <typename T>
    struct has_format_value<T, std::void_t<decltype(format_value(std::declval<format_writer&>(), std::declval<T>(), std::declval<format_options>()))>> {
        static constexpr bool value = true;
    };

    template <typename T> struct type_of { static constexpr format_arg_type value = format_arg_type::unknown; };
#define FORMATXX_TYPE(x, e) template <> struct type_of<x> { static constexpr format_arg_type value = format_arg_type::e; };
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

    template <typename T>
    constexpr result_code format_value_thunk(format_writer& out, void const* ptr, format_options options) {
        format_value(out, *static_cast<T const*>(ptr), options);
        return result_code::success;
    }

    template <typename T>
    constexpr format_arg make_format_arg(T const& value) noexcept {
        constexpr format_arg_type type = type_of<T>::value;

        if constexpr (type != format_arg_type::unknown) {
            return { type, &value };
        }
        else if constexpr (has_format_value<T>::value) {
            return format_arg(&format_value_thunk<T>, &value);
        }
        else if constexpr (std::is_pointer_v<T>) {
            return { format_arg_type::void_pointer, &value };
        }
        else if constexpr (std::is_enum_v<T>) {
            return { type_of<std::underlying_type_t<T>>::value, &value };
        }
        else {
            return {};
        }
    }
}
