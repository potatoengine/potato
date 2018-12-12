// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/gpu/device.h"

namespace gm
{
    class D3d12Device final : public IGPUDevice {
    public:
        virtual ~D3d12Device();

        D3d12Device(D3d12Device&&) = delete;
        D3d12Device& operator=(D3d12Device&) = delete;
    };
}
