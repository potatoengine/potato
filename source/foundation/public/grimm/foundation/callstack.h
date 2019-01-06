// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#pragma once

#include "span.h"
#include "delegate.h"
#include "fixed_string.h"
#include "platform.h"
#include "types.h"
#include <array>

namespace gm {
    struct CallStackRecord {
        fixed_string<128> filename;
        fixed_string<128> symbol;
        uintptr address;
        int line = 0;
    };

    template <std::size_t N = 32>
    using CallStackBuffer = std::array<uintptr, N>;

    struct CallStackReader {
        static span<uintptr> readCallstack(span<uintptr> addresses, uint skip = 0);
        static span<CallStackRecord> tryResolveCallstack(span<uintptr const> addresses, span<CallStackRecord> records);
    };
} // namespace gm
