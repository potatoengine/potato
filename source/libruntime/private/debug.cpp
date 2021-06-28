// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "callstack.h"
#include "debug.h"

#include "potato/spud/string_format.h"
#include "potato/spud/string_writer.h"

#include <array>
#include <iostream>

namespace up::_detail {
    // platform-specific function that must be implemented
    UP_RUNTIME_API UP_NOINLINE FatalErrorAction handleFatalError(
        char const* file,
        int line,
        char const* failedConditionText,
        char const* messageText,
        char const* callstackText);
} // namespace up::_detail

auto up::_detail::raiseFatalError(char const* file, int line, char const* failedConditionText, char const* messageText)
    -> FatalErrorAction {
    constexpr int num_addresses = 64;

    // FIXME: this can be invoked via memory exhaustion, what do?
    string_writer buffer;

    format_to(buffer, "{}({}): ***ASSERTION FAILED*** {}\r\n", file, line, failedConditionText);

    if (messageText != nullptr && *messageText != '\0') {
        format_to(buffer, "{}({}): {}\r\n", file, line, messageText);
    }

    uintptr addresses[num_addresses] = {};

#if !defined(NDEBUG)
    constexpr int num_records = 20;
    callstack::TraceRecord records[num_records] = {};
    auto const stack = callstack::readTrace(addresses);

    auto const resolvedRecords = callstack::resolveTraceRecords(stack, records);
    if (!resolvedRecords.empty()) {
        for (auto const& record : resolvedRecords) {
            format_to(
                buffer,
                "[{:016X}] ({}:{}) {}\r\n",
                record.address,
                record.filename.c_str(),
                record.line,
                record.symbol.c_str());
        }
    }
    else
#endif // !defined(NDEBUG)
    {
        for (auto const addr : addresses) {
            format_to(buffer, "{:016X}\r\n", addr);
        }
    }

    std::cerr << buffer.c_str() << std::flush;

    return _detail::handleFatalError(file, line, failedConditionText, messageText, buffer.data());
}
