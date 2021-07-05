// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format.h"

#include <string>
#include <string_view>

namespace up {
    template <typename StringCharT, typename TraitsT, typename AllocatorT>
    struct formatter<std::basic_string<StringCharT, TraitsT, AllocatorT>> : formatter<string_view> {
        template <typename ContextT>
        void format(std::basic_string<StringCharT, TraitsT, AllocatorT> const& value, ContextT& ctx) {
            formatter<string_view>::format({value.data(), value.size()}, ctx);
        }
    };

    template <typename StringCharT, typename TraitsT>
    struct formatter<std::basic_string_view<StringCharT, TraitsT>> : formatter<string_view> {
        template <typename ContextT>
        void format(std::basic_string_view<StringCharT, TraitsT> const& value, ContextT& ctx) {
            formatter<string_view>::format({value.data(), value.size()}, ctx);
        }
    };
} // namespace up
