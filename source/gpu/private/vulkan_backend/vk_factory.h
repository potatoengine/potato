// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/device.h"
#include "grimm/foundation/box.h"

#if GM_GPU_ENABLE_VULKAN

namespace gm
{
    GM_GPU_API box<IGPUDevice> CreateVulkanFactory();
}

#endif