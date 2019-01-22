// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/platform_windows.h"
#include <d3d11.h>
#include <dxgi1_2.h>

namespace gm::gpu::d3d11 {
    extern zstring_view toNative(Semantic semantic) noexcept;
    extern DXGI_FORMAT toNative(Format format) noexcept;
} // namespace gm::gpu::d3d11
