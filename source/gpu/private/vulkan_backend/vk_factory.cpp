// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_VULKAN

#include "vk_factory.h"

GM_GPU_API auto CreateVulkanGPUFactory() -> box<IGPUDevice> {
    return nullptr;
}

#endif