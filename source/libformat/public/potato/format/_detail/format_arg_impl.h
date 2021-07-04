// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "formatter_float.h"
#include "write_integer.h"
#include "write_string.h"

#include <cinttypes>

template <typename OutputT>
constexpr void up::format_arg::format_into(OutputT& output, string_view spec_string) const {
    using t = _detail::format_value::types;
    switch (_value.type) {
        case t::t_char:
            _detail::write_char(output, _value._char, spec_string);
            break;
        case t::t_int:
            _detail::write_integer(output, _value._int, spec_string);
            break;
        case t::t_unsigned:
            _detail::write_integer(output, _value._unsigned, spec_string);
            break;
        case t::t_longlong:
            _detail::write_integer(output, _value._longlong, spec_string);
            break;
        case t::t_ulonglong:
            _detail::write_integer(output, _value._ulonglong, spec_string);
            break;
        case t::t_float:
            _detail::format_value_to(output, _value._float, spec_string);
            break;
        case t::t_double:
            _detail::format_value_to(output, _value._double, spec_string);
            break;
        case t::t_bool:
            _detail::write_string(output, _value._bool ? "true"_sv : "false"_sv, spec_string);
            break;
        case t::t_cstring:
            _detail::write_string(output, string_view(_value._cstring), spec_string);
            break;
        case t::t_stringview:
            _detail::format_value_to(output, _value._stringview, spec_string);
            break;
        case t::t_voidptr:
            _detail::write_integer(output, reinterpret_cast<std::uintptr_t>(_value._voidptr), spec_string);
            break;
        case t::t_custom:
            _value._custom.thunk(&output, _value._custom.value, spec_string);
            break;
        default:
            break;
    }
}

constexpr int up::format_arg::as_int() const {
    using t = _detail::format_value::types;
    switch (_value.type) {
        case t::t_int:
            return _value._int;
        case t::t_unsigned:
            return static_cast<int>(_value._unsigned);
        case t::t_longlong:
            return static_cast<int>(_value._longlong);
        case t::t_ulonglong:
            return static_cast<int>(_value._ulonglong);
        default:
            return -1;
    }
}
