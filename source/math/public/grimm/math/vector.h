// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/platform.h"

#if defined(GM_ARCH_INTEL)
#    include "_detail/vector_sse.h"
#else
namespace gm {
#    include "packed.h"
    using f32vec4 = f32packed4;

    using vec4 = packed4;
} // namespace gm
#endif
