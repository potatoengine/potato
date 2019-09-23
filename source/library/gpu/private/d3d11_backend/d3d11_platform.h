// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/gpu/common.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/platform_windows.h"
#include <d3d11.h>
#include <dxgi1_2.h>

namespace up::d3d11 {
    extern zstring_view toNative(GpuShaderSemantic semantic) noexcept;
    extern DXGI_FORMAT toNative(GpuFormat format) noexcept;
    extern GpuFormat fromNative(DXGI_FORMAT format) noexcept;
    extern uint32 toByteSize(GpuFormat format) noexcept;
    extern DXGI_FORMAT toNative(GpuIndexFormat type) noexcept;
} // namespace up::d3d11
