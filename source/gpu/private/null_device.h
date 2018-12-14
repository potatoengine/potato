// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/gpu/device.h"

namespace gm
{
    class NullDevice final : public IGPUDevice {
    public:
        NullDevice();
        virtual ~NullDevice();

        NullDevice(NullDevice&&) = delete;
        NullDevice& operator=(NullDevice&) = delete;
    };
}
