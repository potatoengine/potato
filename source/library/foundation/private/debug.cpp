// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/assertion.h"
#include "grimm/foundation/callstack.h"
#include "grimm/foundation/string_format.h"
#include "grimm/foundation/string_writer.h"
#include <spdlog/spdlog.h>

namespace up::_detail {
    // platform-specific function that must be implemented
    UP_FOUNDATION_API UP_NOINLINE error_action platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText);
} // namespace up::_detail

auto up::fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText) -> error_action {
    spdlog::error("{}({}): ***ASSERTION FAILED*** {}", file, line, failedConditionText);

    if (messageText != nullptr && *messageText != '\0') {
        spdlog::error("{}({}): {}", file, line, messageText);
    }

    // FIXME: this can be invoked via memory exhaustion, what do?
    string_writer buffer;
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

    spdlog::error(buffer.c_str());

    return _detail::platform_fatal_error(file, line, failedConditionText, messageText, buffer.data());
}
