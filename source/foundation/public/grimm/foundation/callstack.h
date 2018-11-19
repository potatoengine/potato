// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#pragma once

#include "platform.h"
#include "delegate.h"
#include "string_view.h"
#include "array_view.h"
#include "types.h"

namespace gm
{
    struct CallStackRecord
    {
        string_view filename;
        string_view symbol;
        uintptr address;
        int line = 0;
    };

    struct CallStackReader
    {
        static int readCallstack(array_view<uintptr> addresses, int skip = 0, int max = -1);
        static bool tryResolveCallstack(array_view<uintptr const> addresses, array_view<CallStackRecord> out_records);
    };
}
