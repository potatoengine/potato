// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "platform.h"

/// Break the debugger, if attached, at this point.
#if defined(GM_COMPILER_MICROSOFT)
#    define GM_DEBUG_BREAK() __debugbreak()
#elif defined(GM_PLATFORM_POSIX)
#    include <signal.h>
#    define GM_DEBUG_BREAK() \
        do { \
            raise(SIGTRAP); \
        } while (false)
#else
#    error "GM_DEBUG_BREAK: Unsupported platform/compiler"
#endif

namespace gm {
    /// Results from error_dialog.
    enum class error_action {
        abort,
        debugger_break,
        ignore_once,
        ignore_always,
    };

    /// Ask the user if they should abort, skip the error, or always skip this error.
    GM_FOUNDATION_API GM_NOINLINE error_action fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText);
} // namespace gm

#define _gm_FAIL(failure, message) \
    do { \
        if (static bool _gm_fail_ignore = false; GM_LIKELY(!_gm_fail_ignore)) { \
            switch (::gm::fatal_error(__FILE__, __LINE__, (failure), (message))) { \
            case ::gm::error_action::abort: std::abort(); break; \
            case ::gm::error_action::debugger_break: GM_DEBUG_BREAK(); break; \
            case ::gm::error_action::ignore_once: break; \
            case ::gm::error_action::ignore_always: _gm_fail_ignore = true; break; \
            } \
        } \
    } while (false)
