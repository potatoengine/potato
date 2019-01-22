// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"

#include "_export.h"

namespace gm::gpu {
    class Device;

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(DeviceInfo const&)> callback) = 0;
        virtual box<Device> createDevice(int index) = 0;
    };

    GM_GPU_API box<GpuDeviceFactory> CreateNullGPUFactory();
#if GM_GPU_ENABLE_D3D11
    GM_GPU_API box<GpuDeviceFactory> CreateGPUFactoryD3D11();
#endif
} // namespace gm::gpu
