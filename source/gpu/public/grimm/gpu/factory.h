// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"

#include "_export.h"

namespace gm {
    class GpuDevice;

    struct GpuDeviceInfo {
        int index;
    };

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) = 0;
        virtual box<GpuDevice> createDevice(int index) = 0;
    };

    GM_GPU_API box<GpuDeviceFactory> CreateNullGPUFactory();
#if GM_GPU_ENABLE_D3D12
    GM_GPU_API box<GpuDeviceFactory> CreateD3d12GPUFactory();
#endif
} // namespace gm
