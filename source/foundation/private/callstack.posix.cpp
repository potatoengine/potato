// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#include "callstack.h"
#include "platform.h"
#include "string.h"

#include <execinfo.h>

auto gm::CallStackReader::readCallstack(span<uintptr> addresses, uint skip) -> span<uintptr> {
    void* buffer;

    uint max = addresses.size() - std::min<uint>(addresses.size(), skip);
    uint count = backtrace(&buffer, max);
    skip = std::min(skip, count);

    std::memcpy(addresses.data(), static_cast<uintptr*>(buffer) + skip, std::min(count, max));

    return addresses.first(count - skip);
}

auto gm::CallStackReader::tryResolveCallstack(span<uintptr const> addresses, span<CallStackRecord> records) -> span<CallStackRecord> {
#if !defined(NDEBUG)
    void* const addrs = const_cast<uintptr*>(addresses.data());
    char** symbols = backtrace_symbols(&addrs, addresses.size());

    for (auto index = 0; index != addresses.size(); ++index) {
        CallStackRecord& record = out_records[index];
        record.address = addresses[index];
        record.symbol = string_view(symbols[index]);
    }

    free(symbols);

    return records.first(addresses);
#else
    return {};
#endif // !defined(NDEBUG)
}
