// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "callstack.h"

#include "potato/spud/numeric_util.h"
#include "potato/spud/platform.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"

#include <cstdlib>
#include <cstring>
#include <execinfo.h>

auto up::callstack::readTrace(span<uintptr> addresses, uint skip) -> span<uintptr> {
    void* buffer = nullptr;

    uint max = addresses.size() - min<uint>(addresses.size(), skip);
    uint count = backtrace(&buffer, max);
    skip = min(skip, count);

    std::memcpy(addresses.data(), static_cast<uintptr*>(buffer) + skip, min(count, max));

    return addresses.first(count - skip);
}

auto up::callstack::resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records) -> span<TraceRecord> {
#if !defined(NDEBUG)
    uint max = addresses.size() < records.size() ? addresses.size() : records.size();

    void const* const addrs = addresses.data();
    char** const symbols = backtrace_symbols(&addrs, addresses.size());

    for (auto index : sequence(max)) {
        auto& record = records[index];
        record.address = addresses[index];
        record.symbol = string_view(symbols[index]);
    }

    free(symbols); // NOLINT(cppcoreguidelines-no-malloc)

    return records.first(max);
#else
    return {};
#endif // !defined(NDEBUG)
}
