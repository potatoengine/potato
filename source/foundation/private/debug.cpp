// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "assert.h"
#include "callstack.h"
#include "logging.h"
#include "allocator.h"
#include "string_format.h"

#if !defined(NDEBUG) && !defined(_PREFAST_)

namespace gm::_detail {
	// platform-specific function that must be implemented
	extern GM_API GM_NOINLINE error_action platform_fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText, char const* callstackText);
}

auto gm::fatal_error(char const* file, int line, char const* failedConditionText, char const* messageText) -> error_action
{
	logLine(file, line, LogSeverity::Error, "**ASSERTION FAILED**");
	logFormattedLine(file, line, LogSeverity::Error, "{}({}): {}", file, line, failedConditionText);

	if (messageText != nullptr && messageText[0] != '\0')
		logLine(file, line, LogSeverity::Error, messageText);
	
	format_memory_buffer buffer;
    uintptr addresses[32];
    CallStackRecord records[32];
    CallStackReader reader;
    reader.readCallstack(addresses);

    if (reader.tryResolveCallstack(addresses, records)) 
    {
        for (auto const& record : records)
        {
            format_into(buffer, "{}({}): {}\r\n", record.filename, record.line, record.symbol);
        }
    }

	logFormattedLine(file, line, LogSeverity::Error, {buffer.data(), buffer.size()});

	return _detail::platform_fatal_error(file, line, failedConditionText, messageText, buffer.data());
}

#endif // !defined(NDEBUG)
