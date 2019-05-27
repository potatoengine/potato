// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/core/debug.h"
#include "potato/core/callstack.h"
#include "potato/foundation/string_format.h"
#include "potato/foundation/string_writer.h"
#include <iostream>
#include <array>

namespace up::_detail {
    // platform-specific function that must be implemented
    UP_FOUNDATION_API UP_NOINLINE error_action platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText);
} // namespace up::_detail

auto up::fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText) -> error_action {
    // FIXME: this can be invoked via memory exhaustion, what do?
    string_writer buffer;

    format_into(buffer, "{}({}): ***ASSERTION FAILED*** {}\r\n", file, line, failedConditionText);

    if (messageText != nullptr && *messageText != '\0') {
        format_into(buffer, "{}({}): {}\r\n", file, line, messageText);
    }

    std::array<uintptr, 64> addresses = {};
    std::array<callstack::TraceRecord, 64> records = {};
    auto stack = callstack::readTrace(addresses);

#if !defined(NDEBUG)
    auto resolvedRecords = callstack::resolveTraceRecords(stack, records);
    if (!resolvedRecords.empty()) {
        for (auto const& record : resolvedRecords) {
            format_into(buffer, "[{:016X}] ({}:{}) {}\r\n", record.address, record.filename.c_str(), record.line, record.symbol.c_str());
        }
    }
    else
#endif // !defined(NDEBUG)
    {
        for (uintptr addr : addresses) {
            format_into(buffer, "{:016X}\r\n", addr);
        }
    }

    std::cerr << buffer.c_str() << std::flush;

    return _detail::platform_fatal_error(file, line, failedConditionText, messageText, buffer.data());
}
