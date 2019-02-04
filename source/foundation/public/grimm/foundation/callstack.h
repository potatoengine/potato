// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "span.h"
#include "delegate.h"
#include "fixed_string.h"
#include "platform.h"
#include "types.h"
#include <array>

namespace gm::callstack {
    struct TraceRecord {
        fixed_string<128> filename;
        fixed_string<128> symbol;
        uintptr address;
        int line = 0;
    };

    extern GM_FOUNDATION_API span<uintptr> readTrace(span<uintptr> addresses, uint skip = 0);
    extern GM_FOUNDATION_API span<TraceRecord> resolveTraceRecords(span<uintptr const> addresses, span<TraceRecord> records);
} // namespace gm::callstack
