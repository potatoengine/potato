// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_traits.h"
#include "formatter.h"

#include "potato/spud/concepts.h"
#include "potato/spud/string_view.h"

#include <cinttypes>
#include <initializer_list>
#include <type_traits>

namespace up {
    enum class format_arg_type;

    class format_arg;
    class format_args;

    template <typename Writer, typename T>
    constexpr format_arg make_format_arg(T const& value) noexcept;

    /// Abstraction for a single formattable value
    class format_arg {
    public:
        using thunk_type = void (*)(void*, void const*, up::string_view);

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

        template <typename Writer>
        constexpr void format_into(Writer& output, string_view spec_string = {}) const;

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
        struct map_type {
            using type = T;
        };
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

        template <typename FormatterT>
        concept has_formatter_parse = requires(FormatterT& formatter, format_parse_context& ctx) {
            { formatter.parse(ctx) }
            ->std::convertible_to<decltype(ctx.begin())>;
        };

        template <typename OutputT, typename T>
        constexpr void format_value_to(OutputT& output, T const& value, string_view spec) {
            formatter<T> fmt;
            if constexpr (has_formatter_parse<formatter<T>>) {
                format_parse_context ctx(spec);
                char const* const parsed = fmt.parse(ctx);
                if (parsed != spec.end()) {
                    return;
                }
            }

            fmt.format(output, value);
        }

        template <typename OutputT, typename T>
        constexpr void format_value_thunk(void* output, void const* ptr, string_view spec) {
            format_value_to(*static_cast<OutputT*>(output), *static_cast<T const*>(ptr), spec);
        }
    } // namespace _detail

    template <typename OutputT, formattable T>
    constexpr format_arg make_format_arg(T const& value) noexcept {
        if constexpr (has_formatter_v<T>) {
            return format_arg(&_detail::format_value_thunk<remove_cvref_t<OutputT>, T>, &value);
        }
        else {
            using value_type = typename _detail::map_type<remove_cvref_t<T>>::type;
            return format_arg(static_cast<value_type>(value));
        }
    }

    template <typename OutputT>
    constexpr void format_arg::format_into(OutputT& output, string_view spec_string) const {
        switch (_type) {
            case types::t_char:
                _detail::format_value_to(output, _char, spec_string);
                break;
            case types::t_int:
                _detail::format_value_to(output, _int, spec_string);
                break;
            case types::t_unsigned:
                _detail::format_value_to(output, _unsigned, spec_string);
                break;
            case types::t_longlong:
                _detail::format_value_to(output, _longlong, spec_string);
                break;
            case types::t_ulonglong:
                _detail::format_value_to(output, _ulonglong, spec_string);
                break;
            case types::t_float:
                _detail::format_value_to(output, _float, spec_string);
                break;
            case types::t_double:
                _detail::format_value_to(output, _double, spec_string);
                break;
            case types::t_bool:
                _detail::format_value_to(output, _bool ? "true"_sv : "false"_sv, spec_string);
                break;
            case types::t_cstring:
                _detail::format_value_to(output, string_view(_cstring), spec_string);
                break;
            case types::t_stringview:
                _detail::format_value_to(output, _stringview, spec_string);
                break;
            case types::t_voidptr:
                _detail::format_value_to(output, reinterpret_cast<std::uintptr_t>(_voidptr), spec_string);
                break;
            case types::t_custom:
                _custom.thunk(&output, _custom.value, spec_string);
                break;
            default:
                break;
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
