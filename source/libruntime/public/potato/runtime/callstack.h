// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <potato/spud/span.h>
#include <potato/spud/fixed_string.h>
#include <potato/spud/int_types.h>

namespace up::callstack {
    struct TraceRecord {
        fixed_string<128> filename;
        fixed_string<128> symbol;
        uintptr address = 0;
        int line = 0;
    };

    [[nodiscard]] extern UP_RUNTIME_API auto readTrace(span<uintptr> addresses, uint skip = 0) -> span<uintptr>;
    [[nodiscard]] extern UP_RUNTIME_API auto resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records) -> span<TraceRecord>;
} // namespace up::callstack
