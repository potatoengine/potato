// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "formatter.h"

#include "potato/spud/concepts.h"
#include "potato/spud/string_view.h"

#include <initializer_list>
#include <type_traits>

namespace up {
    namespace _detail_format {
        template <typename T>
        struct type_mapper {};

#define UP_FORMAT_MAP_TYPE(x, e) \
    template <> \
    struct type_mapper<x> { \
        using type = e; \
    };
        UP_FORMAT_MAP_TYPE(char, char);
        UP_FORMAT_MAP_TYPE(signed char, int);
        UP_FORMAT_MAP_TYPE(unsigned char, unsigned);
        UP_FORMAT_MAP_TYPE(signed int, int);
        UP_FORMAT_MAP_TYPE(unsigned int, unsigned);
        UP_FORMAT_MAP_TYPE(signed short, int);
        UP_FORMAT_MAP_TYPE(unsigned short, unsigned);
        UP_FORMAT_MAP_TYPE(signed long, long long);
        UP_FORMAT_MAP_TYPE(unsigned long, unsigned long long);
        UP_FORMAT_MAP_TYPE(signed long long, long long);
        UP_FORMAT_MAP_TYPE(unsigned long long, unsigned long long);
        UP_FORMAT_MAP_TYPE(float, float);
        UP_FORMAT_MAP_TYPE(double, double);
        UP_FORMAT_MAP_TYPE(bool, bool);
        UP_FORMAT_MAP_TYPE(char*, char const*);
        UP_FORMAT_MAP_TYPE(char const*, char const*);
        UP_FORMAT_MAP_TYPE(void*, void const*);
        UP_FORMAT_MAP_TYPE(void const*, void const*);
        UP_FORMAT_MAP_TYPE(::up::string_view, ::up::string_view);
        UP_FORMAT_MAP_TYPE(std::nullptr_t, void const*);
#undef UP_FORMAT_MAP_TYPE

        struct custom_type {
            using thunk_type = void (*)(void const* value, format_parse_context& pctx, void* fctx);

            thunk_type thunk = nullptr;
            void const* value = nullptr;
        };

        template <typename ContextT, typename ValueT>
        constexpr void custom_thunk(void const* value, format_parse_context& pctx, void* fctx);

        template <typename ValueT, typename ContextT, typename FormatterT = formatter<ValueT>>
        constexpr void invoke_format(ValueT const& value, format_parse_context& pctx, ContextT& fctx);

        struct monostate {};

        struct value {
            enum class types {
                t_mono,
                t_int,
                t_unsigned,
                t_longlong,
                t_ulonglong,
                t_char,
                t_float,
                t_double,
                t_bool,
                t_cstring,
                t_stringview,
                t_voidptr,
                t_custom
            };

            constexpr value() noexcept : val_mono() {}
            constexpr value(int value) noexcept : val_int(value), type(types::t_int) {}
            constexpr value(unsigned value) noexcept : val_unsigned(value), type(types::t_unsigned) {}
            constexpr value(long long value) noexcept : val_long_long(value), type(types::t_longlong) {}
            constexpr value(unsigned long long value) noexcept
                : val_unsigned_long_long(value)
                , type(types::t_ulonglong) {}
            constexpr value(char value) noexcept : val_char(value), type(types::t_char) {}
            constexpr value(float value) noexcept : val_float(value), type(types::t_float) {}
            constexpr value(double value) noexcept : val_double(value), type(types::t_double) {}
            constexpr value(bool value) noexcept : val_bool(value), type(types::t_bool) {}
            constexpr value(char const* value) noexcept : val_cstring(value), type(types::t_cstring) {}
            constexpr value(string_view value) noexcept : val_string_view(value), type(types::t_stringview) {}
            constexpr value(void const* value) noexcept : val_voidptr(value), type(types::t_voidptr) {}
            constexpr value(custom_type value) noexcept : val_custom(value), type(types::t_custom) {}

            template <typename VisitorT>
            constexpr decltype(auto) visit(VisitorT&& visitor) const;

            union {
                monostate val_mono;
                int val_int;
                unsigned val_unsigned;
                long long val_long_long;
                unsigned long long val_unsigned_long_long;
                char val_char;
                float val_float;
                double val_double;
                bool val_bool;
                char const* val_cstring;
                string_view val_string_view;
                void const* val_voidptr;
                custom_type val_custom;
            };

            types type = types::t_mono;
        };

        template <size_t N>
        struct value_store {
            value values[N + 1 /* avoid size 0 */];
        };

        template <typename T>
        concept is_value_type = requires(T const& value) {
            ::up::_detail_format::value{value};
        };

        template <typename T>
        concept is_mapped_type = requires {
            typename type_mapper<T>::type;
        };

        template <typename ValueT, typename FormatterT = formatter<ValueT>>
        concept is_formatter_type =
            requires(FormatterT& fmt, format_parse_context& pctx, ValueT const& value, format_context<char*>& fctx) {
            pctx.advance_to(fmt.parse(pctx));
            fmt.format(value, fctx);
        };

        template <typename ContextT, typename ValueT>
        constexpr auto make_format_value(ValueT const& value) noexcept {
            using T = std::decay_t<std::remove_cvref_t<ValueT>>;
            if constexpr (_detail_format::is_value_type<T>) {
                return _detail_format::value(value);
            }
            else if constexpr (_detail_format::is_formatter_type<T>) {
                return _detail_format::custom_type(&_detail_format::custom_thunk<ContextT, T>, &value);
            }
            else if constexpr (_detail_format::is_mapped_type<T>) {
                return _detail_format::value(static_cast<typename type_mapper<T>::type>(value));
            }
            else if constexpr (std::is_enum_v<T>) {
                return _detail_format::value(static_cast<typename type_mapper<std::underlying_type_t<T>>::type>(value));
            }
        };
    } // namespace _detail_format

    /// Abstraction for a single formattable value
    class format_arg {
    public:
        struct handle : _detail_format::custom_type {};

        constexpr format_arg() noexcept = default;
        constexpr explicit format_arg(_detail_format::value value) : _value(value) {}

        constexpr explicit operator bool() const noexcept {
            return _value.type != _detail_format::value::types::t_mono;
        }

        template <typename ContextT>
        constexpr void format(format_parse_context& pctx, ContextT& fctx) const;

        template <class VisitorT>
        friend constexpr decltype(auto) visit_format_arg(VisitorT&& visitor, format_arg const& arg);

    private:
        _detail_format::value _value;
    };

    /// List of format args.
    ///
    /// Only use this type as a temporary value!
    ///
    class format_args {
    public:
        template <size_t N>
        constexpr /*implicit*/ format_args(_detail_format::value_store<N> const& values) noexcept
            : _values(values.values)
            , _count(N) {}

        constexpr format_arg get(int index) const noexcept {
            if (index >= 0 && index < _count) {
                return format_arg(_values[index]);
            }
            return {};
        }

    private:
        _detail_format::value const* _values = nullptr;
        size_t _count = 0;
    };

    template <typename ContextT, typename ValueT>
    constexpr auto make_format_arg(ValueT const& value) noexcept {
        return _detail_format::make_format_value<ContextT>(value);
    }

    template <typename ContextT, typename... Args>
    constexpr auto make_format_args(Args const&... args) noexcept {
        return _detail_format::value_store<sizeof...(Args)>{_detail_format::make_format_value<ContextT>(args)...};
    }

    template <class VisitorT>
    constexpr decltype(auto) visit_format_arg(VisitorT&& visitor, format_arg const& arg) {
        return arg._value.visit(static_cast<decltype(visitor)>(visitor));
    }

    template <typename ContextT>
    constexpr void format_arg::format(format_parse_context& pctx, ContextT& fctx) const {
        using types = _detail_format::value::types;
        switch (_value.type) {
            case types::t_char:
                return _detail_format::invoke_format(_value.val_char, pctx, fctx);
            case types::t_int:
                return _detail_format::invoke_format(_value.val_int, pctx, fctx);
            case types::t_unsigned:
                return _detail_format::invoke_format(_value.val_unsigned, pctx, fctx);
            case types::t_longlong:
                return _detail_format::invoke_format(_value.val_long_long, pctx, fctx);
            case types::t_ulonglong:
                return _detail_format::invoke_format(_value.val_unsigned_long_long, pctx, fctx);
            case types::t_float:
                return _detail_format::invoke_format(_value.val_float, pctx, fctx);
            case types::t_double:
                return _detail_format::invoke_format(_value.val_double, pctx, fctx);
            case types::t_bool:
                return _detail_format::invoke_format(_value.val_bool ? "true"_sv : "false"_sv, pctx, fctx);
            case types::t_cstring:
                return _detail_format::invoke_format(string_view(_value.val_cstring), pctx, fctx);
            case types::t_stringview:
                return _detail_format::invoke_format(_value.val_string_view, pctx, fctx);
            case types::t_voidptr:
                return _detail_format::invoke_format(reinterpret_cast<std::uintptr_t>(_value.val_voidptr), pctx, fctx);
            case types::t_custom:
                return _value.val_custom.thunk(_value.val_custom.value, pctx, &fctx);
            default:
                return;
        }
    }

    template <class VisitorT>
    constexpr decltype(auto) _detail_format::value::visit(VisitorT&& visitor) const {
        switch (type) {
            case types::t_char:
                return visitor(val_char);
            case types::t_int:
                return visitor(val_int);
            case types::t_unsigned:
                return visitor(val_unsigned);
            case types::t_longlong:
                return visitor(val_long_long);
            case types::t_ulonglong:
                return visitor(val_unsigned_long_long);
            case types::t_float:
                return visitor(val_float);
            case types::t_double:
                return visitor(val_double);
            case types::t_bool:
                return visitor(val_bool);
            case types::t_cstring:
                return visitor(val_cstring);
            case types::t_stringview:
                return visitor(val_string_view);
            case types::t_voidptr:
                return visitor(val_voidptr);
            case types::t_custom:
                return visitor(val_custom);
            default:
                return visitor(monostate{});
        }
    }

    template <typename ValueT, typename ContextT, typename FormatterT>
    constexpr void _detail_format::invoke_format(ValueT const& value, format_parse_context& pctx, ContextT& fctx) {
        FormatterT fmt;
        pctx.advance_to(fmt.parse(pctx));
        fmt.format(value, fctx);
    }

    template <typename ContextT, typename ValueT>
    constexpr void _detail_format::custom_thunk(void const* value, format_parse_context& pctx, void* fctx) {
        return invoke_format(*static_cast<ValueT const*>(value), pctx, *static_cast<ContextT*>(fctx));
    }
} // namespace up
