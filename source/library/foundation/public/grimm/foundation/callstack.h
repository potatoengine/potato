// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "span.h"
#include "fixed_string.h"
#include "int_types.h"

namespace up::callstack {
    struct TraceRecord {
        fixed_string<128> filename;
        fixed_string<128> symbol;
        uintptr address;
        int line = 0;
    };

    extern UP_FOUNDATION_API span<uintptr> readTrace(span<uintptr> addresses, uint skip = 0);
    extern UP_FOUNDATION_API span<TraceRecord> resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records);
} // namespace up::callstack
