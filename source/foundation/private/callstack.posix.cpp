// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#include "callstack.h"
#include "platform.h"
#include "string.h"

#include <execinfo.h>

int gm::CallStackReader::readCallstack(array_view<uintptr> addresses, int skip, int max)
{
    return backtrace(&addresses.data(), std::min(size, addresses.size()));
}

bool gm::CallStackReader::tryResolveCallstack(array_view<uintptr const> addresses, array_view<CallStackRecord> out_records)
{
#if !defined(NDEBUG)
    char** symbols = backtrace_symbols(&addresses.data(), addresses.size());

    for (auto index = 0; index != addresses.size(); ++index)
    {
        CallStackRecord& record = out_records[index];
        record.symbol = string_view(symbols[index]);
    }

    free(symbols);

    return true;
#else
    return false;
#endif // !defined(NDEBUG)
}
