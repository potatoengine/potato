// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

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
    constexpr int num_addresses = 64;

    // FIXME: this can be invoked via memory exhaustion, what do?
    string_writer buffer;

    format_append(buffer, "{}({}): ***ASSERTION FAILED*** {}\r\n", file, line, failedConditionText);

    if (messageText != nullptr && *messageText != '\0') {
        format_append(buffer, "{}({}): {}\r\n", file, line, messageText);
    }

    std::array<uintptr, num_addresses> addresses = {};

#if !defined(NDEBUG)
    constexpr int num_records = 20;
    auto records = std::array<callstack::TraceRecord, num_records>{};
    auto const stack = callstack::readTrace(addresses);

    auto const resolvedRecords = callstack::resolveTraceRecords(stack, records);
    if (!resolvedRecords.empty()) {
        for (auto const& record : resolvedRecords) {
            format_append(buffer, "[{:016X}] ({}:{}) {}\r\n", record.address, record.filename.c_str(), record.line, record.symbol.c_str());
        }
    }
    else
#endif // !defined(NDEBUG)
    {
        for (auto const addr : addresses) {
            format_append(buffer, "{:016X}\r\n", addr);
        }
    }

    std::cerr << buffer.c_str() << std::flush;

    return _detail::handleFatalError(file, line, failedConditionText, messageText, buffer.data());
}
