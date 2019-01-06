// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#pragma once

#include "span.h"
#include "delegate.h"
#include "fixed_string.h"
#include "platform.h"
#include "types.h"

namespace gm {
    struct CallStackRecord {
        fixed_string<128> filename;
        fixed_string<128> symbol;
        uintptr address;
        int line = 0;
    };

    struct CallStackReader {
        static uint readCallstack(span<uintptr> addresses, uint skip = 0);
        static bool tryResolveCallstack(span<uintptr const> addresses, span<CallStackRecord> out_records);
    };
} // namespace gm
