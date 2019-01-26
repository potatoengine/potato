// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"
#include "grimm/foundation/rc.h"

#include "_export.h"

namespace gm::gpu {
    class Device;

    class Factory {
    public:
        Factory() = default;
        virtual ~Factory() = default;

        Factory(Factory&&) = delete;
        Factory& operator=(Factory&&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(DeviceInfo const&)> callback) = 0;
        virtual rc<Device> createDevice(int index) = 0;
    };

    GM_GPU_API box<Factory> CreateFactoryNull();
#if defined(GM_GPU_ENABLE_D3D11)
    GM_GPU_API box<Factory> CreateFactoryD3D11();
#endif
} // namespace gm::gpu
