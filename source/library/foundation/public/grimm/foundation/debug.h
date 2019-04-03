// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "platform.h"

#include <cstdlib>

/// Break the debugger, if attached, at this point.
#if defined(UP_COMPILER_MICROSOFT)
#    define UP_DEBUG_BREAK() __debugbreak()
#elif defined(UP_PLATFORM_POSIX)
#    include <signal.h>
#    define UP_DEBUG_BREAK() \
        do { \
            raise(SIGTRAP); \
        } while (false)
#else
#    error "UP_DEBUG_BREAK: Unsupported platform/compiler"
#endif

namespace up {
    /// Results from error_dialog.
    enum class error_action {
        abort,
        debugger_break,
        ignore_once,
        ignore_always,
    };

    /// Ask the user if they should abort, skip the error, or always skip this error.
    UP_FOUNDATION_API UP_NOINLINE error_action fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText);
} // namespace up

#define _up_FAIL(failure, message) \
    do { \
        if (static bool _up_fail_ignore = false; UP_LIKELY(!_up_fail_ignore)) { \
            switch (::up::fatal_error(__FILE__, __LINE__, (failure), (message))) { \
            case ::up::error_action::abort: std::abort(); break; \
            case ::up::error_action::debugger_break: UP_DEBUG_BREAK(); break; \
            case ::up::error_action::ignore_once: break; \
            case ::up::error_action::ignore_always: _up_fail_ignore = true; break; \
            } \
        } \
    } while (false)
