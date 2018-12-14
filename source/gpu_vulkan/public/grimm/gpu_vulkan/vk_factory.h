// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/device.h"

namespace gm
{
    GM_GPU_VULKAN_API box<IGPUDevice> CreateVulkanFactory();
}
