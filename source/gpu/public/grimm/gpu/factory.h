// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"

#include "_export.h"
#include "device_info.h"

namespace gm {
    class GpuDevice;

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(DeviceInfo const&)> callback) = 0;
        virtual box<GpuDevice> createDevice(int index) = 0;
    };

    GM_GPU_API box<GpuDeviceFactory> CreateNullGPUFactory();
#if GM_GPU_ENABLE_VULKAN
    GM_GPU_API box<GpuDeviceFactory> CreateVulkanGPUFactory();
#endif
#if GM_GPU_ENABLE_D3D12
    GM_GPU_API box<GpuDeviceFactory> CreateD3d12GPUFactory();
#endif
} // namespace gm
