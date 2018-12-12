// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm
{
    class IGPUDevice {
    public:
        virtual ~IGPUDevice();

        IGPUDevice(IGPUDevice&&) = delete;
        IGPUDevice& operator=(IGPUDevice&) = delete;
    };
}
