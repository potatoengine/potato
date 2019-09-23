// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/debug.h"
#include "potato/runtime/callstack.h"
#include "potato/spud/string_format.h"
#include "potato/spud/string_writer.h"
#include <iostream>
#include <array>

namespace up::_detail {
    // platform-specific function that must be implemented
    UP_RUNTIME_API UP_NOINLINE FatalErrorAction handleFatalError(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText);
} // namespace up::_detail

auto up::_detail::raiseFatalError(char const* file, int line, char const* failedConditionText, char const* messageText) -> FatalErrorAction {
    // FIXME: this can be invoked via memory exhaustion, what do?
    string_writer buffer;

    format_into(buffer, "{}({}): ***ASSERTION FAILED*** {}\r\n", file, line, failedConditionText);

    if (messageText != nullptr && *messageText != '\0') {
        format_into(buffer, "{}({}): {}\r\n", file, line, messageText);
    }

    std::array<uintptr, 64> addresses = {};

#if !defined(NDEBUG)
    std::array<callstack::TraceRecord, 64> records = {};
    auto stack = callstack::readTrace(addresses);

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

    return _detail::handleFatalError(file, line, failedConditionText, messageText, buffer.data());
}
