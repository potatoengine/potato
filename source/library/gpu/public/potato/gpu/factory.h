// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/foundation/box.h"
#include "potato/foundation/delegate.h"
#include "potato/foundation/rc.h"

#include "_export.h"

namespace up::gpu {
    class GpuDevice;

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(DeviceInfo const&)> callback) = 0;
        virtual rc<GpuDevice> createDevice(int index) = 0;
    };

    UP_GPU_API box<GpuDeviceFactory> CreateFactoryNull();
#if defined(UP_GPU_ENABLE_D3D11)
    UP_GPU_API box<GpuDeviceFactory> CreateFactoryD3D11();
#endif
} // namespace up::gpu
