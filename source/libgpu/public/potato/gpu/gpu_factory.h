// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "gpu_common.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/rc.h"

#include "_export.h"

namespace up {
    class GpuDevice;

    class GpuDeviceFactory {
    public:
        GpuDeviceFactory() = default;
        virtual ~GpuDeviceFactory() = default;

        GpuDeviceFactory(GpuDeviceFactory&&) = delete;
        GpuDeviceFactory& operator=(GpuDeviceFactory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) = 0;
        virtual rc<GpuDevice> createDevice(int index) = 0;
    };

    UP_GPU_API box<GpuDeviceFactory> CreateFactoryNull();
#if defined(UP_GPU_ENABLE_D3D11)
    UP_GPU_API box<GpuDeviceFactory> CreateFactoryD3D11();
#endif
} // namespace up
