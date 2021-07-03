// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "write_float.h"
#include "write_integer.h"
#include "write_string.h"

#include <cinttypes>

template <typename Writer>
constexpr up::format_result up::format_arg::format_into(Writer& output, string_view spec_string) const {
    switch (_type) {
        case format_arg_type::char_t:
            _detail::write_char(output, *static_cast<char const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::signed_char:
            _detail::write_integer(output, *static_cast<signed char const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::unsigned_char:
            _detail::write_integer(output, *static_cast<unsigned char const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::signed_int:
            _detail::write_integer(output, *static_cast<signed int const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::unsigned_int:
            _detail::write_integer(output, *static_cast<unsigned int const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::signed_short_int:
            _detail::write_integer(output, *static_cast<signed short const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::unsigned_short_int:
            _detail::write_integer(output, *static_cast<unsigned short const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::signed_long_int:
            _detail::write_integer(output, *static_cast<signed long const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::unsigned_long_int:
            _detail::write_integer(output, *static_cast<unsigned long const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::signed_long_long_int:
            _detail::write_integer(output, *static_cast<signed long long const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::unsigned_long_long_int:
            _detail::write_integer(output, *static_cast<unsigned long long const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::single_float:
            _detail::write_float(output, *static_cast<float const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::double_float:
            _detail::write_float(output, *static_cast<double const*>(_value), spec_string);
            return format_result::success;
        case format_arg_type::boolean:
            _detail::write_string(output, *static_cast<bool const*>(_value) ? "true"_sv : "false"_sv, spec_string);
            return format_result::success;
        case format_arg_type::char_string:
            _detail::write_string(output, string_view(*static_cast<char const* const*>(_value)), spec_string);
            return format_result::success;
        case format_arg_type::null_pointer:
            _detail::write_string(output, "nullptr"_sv, spec_string);
            return format_result::success;
        case format_arg_type::void_pointer:
            _detail::write_integer(
                output,
                reinterpret_cast<std::uintptr_t>(*static_cast<void const* const*>(_value)),
                spec_string);
            return format_result::success;
        case format_arg_type::custom:
            return _thunk(&output, _value);
        default:
            return format_result::success;
    }
}

constexpr int up::format_arg::as_int() const {
    switch (_type) {
        case format_arg_type::char_t:
            return static_cast<int>(*static_cast<char const*>(_value));
        case format_arg_type::signed_char:
            return static_cast<int>(*static_cast<signed char const*>(_value));
        case format_arg_type::unsigned_char:
            return static_cast<int>(*static_cast<unsigned char const*>(_value));
        case format_arg_type::signed_int:
            return static_cast<int>(*static_cast<signed int const*>(_value));
        case format_arg_type::unsigned_int:
            return static_cast<int>(*static_cast<unsigned int const*>(_value));
        case format_arg_type::signed_short_int:
            return static_cast<int>(*static_cast<signed short const*>(_value));
        case format_arg_type::unsigned_short_int:
            return static_cast<int>(*static_cast<unsigned short const*>(_value));
        case format_arg_type::signed_long_int:
            return static_cast<int>(*static_cast<signed long const*>(_value));
        case format_arg_type::unsigned_long_int:
            return static_cast<int>(*static_cast<unsigned long const*>(_value));
        case format_arg_type::signed_long_long_int:
            return static_cast<int>(*static_cast<signed long long const*>(_value));
        case format_arg_type::unsigned_long_long_int:
            return static_cast<int>(*static_cast<unsigned long long const*>(_value));
        default:
            return -1;
    }
}
