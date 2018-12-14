// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/device.h"

namespace gm
{
    class VkDevice final : public IGPUDevice {
    public:
        virtual ~VkDevice();

        VkDevice(VkDevice&&) = delete;
        VkDevice& operator=(VkDevice&) = delete;
    };
}
