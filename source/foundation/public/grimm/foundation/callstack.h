// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#pragma once

#include "array_view.h"
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
        static uint readCallstack(array_view<uintptr> addresses, uint skip = 0);
        static bool tryResolveCallstack(array_view<uintptr const> addresses, array_view<CallStackRecord> out_records);
    };
} // namespace gm
