// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/factory.h"

namespace gm {
    class VknFactory final : public IGPUFactory {
    public:
        virtual ~VknFactory();

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<IGPUDevice> createDevice(int index) override;
    };
} // namespace gm
