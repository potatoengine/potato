// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "device.h"

namespace gm {
    class NullDevice final : public IGPUDevice {
    public:
        NullDevice();
        virtual ~NullDevice();

        NullDevice(NullDevice&&) = delete;
        NullDevice& operator=(NullDevice&) = delete;

        box<ISwapChain> createSwapChain(void* native_window) override;
        box<IDescriptorHeap> createDescriptorHeap() override;
    };
} // namespace gm
