// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <potato/runtime/debug.h>
#include <potato/foundation/string_format.h>
#include <potato/foundation/fixed_string_writer.h>
#include <utility>

#if defined(NDEBUG)

#    define UP_ASSERT(condition, ...) \
        do { \
        } while (false)
#    define UP_UNREACHABLE(...) \
        do { \
        } while (false)

#elif _PREFAST_ // Microsoft's /analyze tool

#    define UP_ASSERT(condition, ...) __analysis_assume(condition)
#    define UP_UNREACHABLE(...) __analysis_assume(false)

#else

namespace up::_detail {
    // abstraction to deal with assert instances that don't have a message at all
    template <typename Buffer, typename... Args>
    void constexpr formatAssertion(Buffer& buffer, char const* format, Args&&... args) { format_into(buffer, format, std::forward<Args>(args)...); }
    template <typename Buffer>
    void constexpr formatAssertion(Buffer&) {}
} // namespace up::_detail

#    define _up_FORMAT_FAIL(condition_text, ...) \
        do { \
            ::up::fixed_string_writer<512> _up_fail_buffer; \
            ::up::_detail::formatAssertion(_up_fail_buffer, ##__VA_ARGS__); \
            _up_FAIL((condition_text), _up_fail_buffer.c_str()); \
        } while (false)

#    define UP_ASSERT(condition, ...) \
        do { \
            if (UP_UNLIKELY(!((condition)))) { \
                _up_FORMAT_FAIL(#condition, ##__VA_ARGS__); \
            } \
        } while (false)

#    define UP_UNREACHABLE(...) _up_FAIL("unreachable code", ##__VA_ARGS__)

#endif // _PREFAST_
