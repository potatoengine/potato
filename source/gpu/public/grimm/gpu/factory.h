// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/delegate.h"

#include "device_info.h"

namespace gm
{
    class IGPUDevice;

    class IGPUFactory {
    public:
        IGPUFactory() = default;
        virtual ~IGPUFactory();

        IGPUFactory(IGPUFactory&&) = delete;
        IGPUFactory& operator=(IGPUFactory&) = delete;

        virtual bool isEnabled() const = 0;
        virtual void enumerateDevices(delegate<void(DeviceInfo const&)> callback) = 0;
        virtual box<IGPUDevice> createDevice(int index) = 0;
    };
}
