// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/factory.h"

#if GM_GPU_ENABLE_VULKAN

namespace gm
{
    class VkFactory final : public IGPUFactory {
    public:
    };
}

#endif