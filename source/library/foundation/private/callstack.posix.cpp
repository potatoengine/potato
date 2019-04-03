// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/callstack.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/string.h"
#include "grimm/foundation/numeric_util.h"

#include <cstring>
#include <cstdlib>
#include <execinfo.h>

auto up::callstack::readTrace(span<uintptr> addresses, uint skip) -> span<uintptr> {
    void* buffer;

    uint max = addresses.size() - min<uint>(addresses.size(), skip);
    uint count = backtrace(&buffer, max);
    skip = min(skip, count);

    std::memcpy(addresses.data(), static_cast<uintptr*>(buffer) + skip, min(count, max));

    return addresses.first(count - skip);
}

auto up::callstack::resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records) -> span<TraceRecord> {
#if !defined(NDEBUG)
    uint max = addresses.size() < records.size() ? addresses.size() : records.size();

    void* const addrs = const_cast<uintptr*>(addresses.data());
    char** symbols = backtrace_symbols(&addrs, addresses.size());

    for (auto index = 0; index != max; ++index) {
        auto& record = records[index];
        record.address = addresses[index];
        record.symbol = string_view(symbols[index]);
    }

    free(symbols);

    return records.first(max);
#else
    return {};
#endif // !defined(NDEBUG)
}
