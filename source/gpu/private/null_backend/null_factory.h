// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "factory.h"

namespace gm {
    class NullDevice;

    class NullFactory final : public IGPUFactory {
    public:
        NullFactory();
        virtual ~NullFactory();

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<IGPUDevice> createDevice(int index) override;
    };
} // namespace gm
