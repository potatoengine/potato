// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "debug.h"
#include "string_view.h"
#include <iterator>

// needed by fmtlib
#if !defined(FMT_ASSERT)
#    define FMT_ASSERT(cond, message) \
        do { \
            if (!(cond)) { \
                ::gm::fatal_error(__FILE__, __LINE__, #cond, (message)); \
                std::abort(); \
            } \
        } while (false)
#endif
#if !defined(assert)
#    define assert(cond) \
        FMT_ASSERT((cond), "assertion failed")
#endif

#include <fmt/format.h>

namespace gm {
    template <typename... Args>
    constexpr decltype(auto) format(string_view format, Args const&... args) {
        return ::fmt::vformat(::fmt::string_view(format.data(), format.size()), ::fmt::make_format_args(args...));
    }

    template <typename Buffer, typename... Args>
    constexpr decltype(auto) format_into(Buffer& buffer, string_view format, Args const&... args) {
        return ::fmt::format_to(std::back_inserter(buffer), ::fmt::string_view(format.data(), format.size()), args...);
    }
} // namespace gm
