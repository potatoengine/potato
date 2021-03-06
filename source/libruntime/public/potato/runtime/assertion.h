// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "debug.h"

#include "potato/format/format.h"

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
    template <typename Writer, typename... Args>
    void constexpr formatAssertion(Writer& writer, char const* format, Args&&... args) {
        format_to(writer, format, std::forward<Args>(args)...);
    }
    template <typename Writer>
    void constexpr formatAssertion(Writer&) {}
} // namespace up::_detail

#    define uppriv_FORMAT_FAIL(condition_text, ...) \
        do { \
            char uppriv_fail_buffer[512] = { \
                0, \
            }; \
            ::up::fixed_writer uppriv_fail_writer(uppriv_fail_buffer); \
            ::up::_detail::formatAssertion(uppriv_fail_writer, ##__VA_ARGS__); \
            uppriv_FAIL((condition_text), uppriv_fail_buffer); \
        } while (false)

#    define UP_ASSERT(condition, ...) \
        do { \
            if (UP_UNLIKELY(!((condition)))) { \
                uppriv_FORMAT_FAIL(#condition, ##__VA_ARGS__); \
            } \
        } while (false)

#    define UP_UNREACHABLE(...) uppriv_FAIL("unreachable code", ##__VA_ARGS__)

#endif // _PREFAST_
