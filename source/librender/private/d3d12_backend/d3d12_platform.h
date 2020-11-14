// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "gpu_common.h"

#include "potato/spud/platform_windows.h"
#include "potato/spud/zstring_view.h"
#include "potato/runtime/com_ptr.h"

#include <d3d12.h>
#include <dxgi1_6.h>

// helper global defines 
using IDXGIFactoryType = IDXGIFactory2;
using IDXGIFactoryPtr = up::com_ptr<IDXGIFactoryType>;
using IDXGIAdapterType = IDXGIAdapter1;
using IDXGIAdapterPtr = up::com_ptr<IDXGIAdapterType>;
using IDXGISwapChainType = IDXGISwapChain1;
using IDXGISwapChainPtr = up::com_ptr<IDXGISwapChainType>;

using ID3DDeviceType = ID3D12Device;
using ID3DDevicePtr = up::com_ptr<ID3D12Device>;
using ID3DResourceType = ID3D12Resource;
using ID3DResourcePtr = up::com_ptr<ID3D12Resource>;

using ID3DCommandAllocatorPtr = up::com_ptr<ID3D12CommandAllocator>;

using ID3DCommandListPtr = up::com_ptr<ID3D12GraphicsCommandList>;
using ID3DCommandQueuePtr = up::com_ptr<ID3D12CommandQueue>;

namespace up::d3d12 {
    extern zstring_view toNative(GpuShaderSemantic semantic) noexcept;
    extern DXGI_FORMAT toNative(GpuFormat format) noexcept;
    extern GpuFormat fromNative(DXGI_FORMAT format) noexcept;
    extern uint32 toByteSize(GpuFormat format) noexcept;
    extern DXGI_FORMAT toNative(GpuIndexFormat type) noexcept;
} // namespace up::d3d12
