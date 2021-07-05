// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "formatter.h"

#include "potato/spud/concepts.h"
#include "potato/spud/string_view.h"

#include <initializer_list>
#include <type_traits>

namespace up {
    /// Abstraction for a single formattable value
    class format_arg {
    public:
        using thunk_type = void (*)(void*, void const*, format_parse_context& ctx);

        constexpr format_arg() noexcept {} // clang disallows =default here
        constexpr format_arg(int value) noexcept : _int(value), _type(types::t_int) {}
        constexpr format_arg(unsigned value) noexcept : _unsigned(value), _type(types::t_unsigned) {}
        constexpr format_arg(long long value) noexcept : _longlong(value), _type(types::t_longlong) {}
        constexpr format_arg(unsigned long long value) noexcept : _ulonglong(value), _type(types::t_ulonglong) {}
        constexpr format_arg(char value) noexcept : _char(value), _type(types::t_char) {}
        constexpr format_arg(float value) noexcept : _float(value), _type(types::t_float) {}
        constexpr format_arg(double value) noexcept : _double(value), _type(types::t_double) {}
        constexpr format_arg(bool value) noexcept : _bool(value), _type(types::t_bool) {}
        constexpr format_arg(char const* value) noexcept : _cstring(value), _type(types::t_cstring) {}
        constexpr format_arg(string_view value) noexcept : _stringview(value), _type(types::t_stringview) {}
        constexpr format_arg(void const* value) noexcept : _voidptr(value), _type(types::t_voidptr) {}
        constexpr format_arg(thunk_type thunk, void const* value) noexcept
            : _custom({thunk, value})
            , _type(types::t_custom) {}

        template <typename OutputT>
        constexpr void format_into(OutputT& output, format_parse_context& ctx) const;

        constexpr int as_int() const noexcept;

    private:
        enum class types {
            t_void,
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

        struct custom {
            thunk_type thunk = nullptr;
            void const* value = nullptr;
        };

        union {
            int _int = 0;
            unsigned _unsigned;
            long long _longlong;
            unsigned long long _ulonglong;
            char _char;
            float _float;
            double _double;
            bool _bool;
            char const* _cstring;
            string_view _stringview;
            void const* _voidptr;
            custom _custom;
        };

        types _type = types::t_void;
    };

    /// List of format args.
    ///
    /// Only use this type as a temporary value!
    ///
    class format_args {
    public:
        /*implicit*/ format_args(std::initializer_list<format_arg> args) noexcept
            : _args(args.begin())
            , _count(args.size()) {}

        constexpr format_arg get(int index) const noexcept {
            format_arg result;
            if (index >= 0 && index < _count) {
                result = _args[index];
            }
            return result;
        }

    private:
        format_arg const* _args = nullptr;
        size_t _count = 0;
    };

    namespace _detail {
        template <typename T>
        struct map_type {};
#define UP_FORMAT_MAP_TYPE(x, e) \
    template <> \
    struct map_type<x> { \
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
        UP_FORMAT_MAP_TYPE(::up::string_view, ::up::string_view);
        UP_FORMAT_MAP_TYPE(std::nullptr_t, void const*);
#undef UP_FORMAT_MAP_TYPE

        template <convertible_to<string_view> T>
        struct map_type<T> {
            using type = string_view;
        };

        template <typename T>
        struct map_type<T*> {
            using type = void const*;
        };

        template <enumeration T>
        struct map_type<T> {
            using type = std::underlying_type_t<T>;
        };

        template <typename ValueT>
        using format_mapped_t = typename _detail::map_type<remove_cvref_t<ValueT>>::type;

        template <typename OutputT, typename T>
        constexpr void format_value_to(OutputT& output, T const& value, format_parse_context& ctx) {
            formatter<T> fmt;
            ctx.advance_to(fmt.parse(ctx));
            format_format_context fctx(output);
            fmt.format(value, fctx);
        }

        template <typename OutputT, typename T>
        constexpr void format_value_thunk(void* output, void const* ptr, format_parse_context& ctx) {
            return format_value_to(*static_cast<OutputT*>(output), *static_cast<T const*>(ptr), ctx);
        }
    } // namespace _detail

    template <typename OutputT, formatter_enabled ValueT>
    constexpr format_arg make_format_arg(ValueT const& value) noexcept {
        return format_arg(&_detail::format_value_thunk<remove_cvref_t<OutputT>, ValueT>, &value);
    }

    template <typename OutputT, typename ValueT, typename = std::void_t<_detail::format_mapped_t<ValueT>>>
    constexpr format_arg make_format_arg(ValueT const& value) noexcept {
        return format_arg(static_cast<_detail::format_mapped_t<ValueT>>(value));
    }

    template <typename OutputT>
    constexpr void format_arg::format_into(OutputT& output, format_parse_context& ctx) const {
        switch (_type) {
            case types::t_char:
                return _detail::format_value_to(output, _char, ctx);
            case types::t_int:
                return _detail::format_value_to(output, _int, ctx);
            case types::t_unsigned:
                return _detail::format_value_to(output, _unsigned, ctx);
            case types::t_longlong:
                return _detail::format_value_to(output, _longlong, ctx);
            case types::t_ulonglong:
                return _detail::format_value_to(output, _ulonglong, ctx);
            case types::t_float:
                return _detail::format_value_to(output, _float, ctx);
            case types::t_double:
                return _detail::format_value_to(output, _double, ctx);
            case types::t_bool:
                return _detail::format_value_to(output, _bool ? "true"_sv : "false"_sv, ctx);
            case types::t_cstring:
                return _detail::format_value_to(output, string_view(_cstring), ctx);
            case types::t_stringview:
                return _detail::format_value_to(output, _stringview, ctx);
            case types::t_voidptr:
                return _detail::format_value_to(output, reinterpret_cast<std::uintptr_t>(_voidptr), ctx);
            case types::t_custom:
                return _custom.thunk(&output, _custom.value, ctx);
            default:
                return;
        }
    }

    constexpr int format_arg::as_int() const noexcept {
        switch (_type) {
            case types::t_int:
                return _int;
            case types::t_unsigned:
                return static_cast<int>(_unsigned);
            case types::t_longlong:
                return static_cast<int>(_longlong);
            case types::t_ulonglong:
                return static_cast<int>(_ulonglong);
            default:
                return -1;
        }
    }

} // namespace up
