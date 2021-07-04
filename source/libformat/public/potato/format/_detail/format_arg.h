// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_traits.h"
#include "formatter.h"

#include "potato/spud/string_view.h"

#include <initializer_list>
#include <type_traits>

namespace up {
    enum class format_arg_type;

    class format_arg;
    class format_args;

    template <typename Writer, typename T>
    constexpr format_arg make_format_arg(T const& value) noexcept;

    namespace _detail {
        struct format_value {
            struct custom {
                using thunk_type = void (*)(void*, void const*, up::string_view);

                thunk_type thunk = nullptr;
                void const* value = nullptr;
            };

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

            constexpr format_value() noexcept {}
            constexpr format_value(int value) noexcept : _int(value), type(types::t_int) {}
            constexpr format_value(unsigned value) noexcept : _unsigned(value), type(types::t_unsigned) {}
            constexpr format_value(long long value) noexcept : _longlong(value), type(types::t_longlong) {}
            constexpr format_value(unsigned long long value) noexcept : _ulonglong(value), type(types::t_ulonglong) {}
            constexpr format_value(char value) noexcept : _char(value), type(types::t_char) {}
            constexpr format_value(float value) noexcept : _float(value), type(types::t_float) {}
            constexpr format_value(double value) noexcept : _double(value), type(types::t_double) {}
            constexpr format_value(bool value) noexcept : _bool(value), type(types::t_bool) {}
            constexpr format_value(char const* value) noexcept : _cstring(value), type(types::t_cstring) {}
            constexpr format_value(string_view value) noexcept : _stringview(value), type(types::t_stringview) {}
            constexpr format_value(void const* value) noexcept : _voidptr(value), type(types::t_voidptr) {}
            constexpr format_value(custom value) noexcept : _custom(value), type(types::t_custom) {}

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

            types type = types::t_void;
        };
    } // namespace _detail

    /// Abstraction for a single formattable value
    class format_arg {
    public:
        constexpr format_arg() noexcept = default;
        constexpr format_arg(_detail::format_value value) noexcept : _value(value) {}

        template <typename Writer>
        constexpr void format_into(Writer& output, string_view spec_string = {}) const;

        constexpr int as_int() const;

    private:
        _detail::format_value _value;
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

        template <typename OutputT, typename T>
        constexpr void format_value_to(OutputT& output, T const& value, string_view spec) {
            formatter<T> fmt;
            if constexpr (has_formatter_parse<formatter<T>>) {
                char const* const parsed = fmt.parse(spec);
                if (parsed != spec.end()) {
                    return;
                }
            }

            fmt.format(output, value);
        }

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
    } // namespace _detail

    template <typename OutputT, formattable T>
    constexpr format_arg make_format_arg(T const& value) noexcept {
        if constexpr (_detail::has_format_value<OutputT, T>) {
            _detail::format_value::custom custom;
            custom.thunk = &_detail::format_value_thunk<remove_cvref_t<OutputT>, T>;
            custom.value = &value;
            return format_arg(custom);
        }
        else if constexpr (std::is_convertible_v<T, up::string_view>) {
            return format_arg(string_view(value));
        }
        else if constexpr (std::is_enum_v<T>) {
            using value_type = typename _detail::map_type<std::underlying_type_t<T>>::type;
            return format_arg(static_cast<value_type>(value));
        }
        else {
            using value_type = typename _detail::map_type<remove_cvref_t<T>>::type;
            return format_arg(static_cast<value_type>(value));
        }
    }
} // namespace up
