// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_VULKAN

#include "vk_factory.h"
#include "device.h"

auto gm::CreateVulkanGPUFactory() -> box<IGPUDevice> {
    return nullptr;
}

bool gm::VkFactory::isEnabled() const {
    return false;
}

void gm::VkFactory::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
}

auto gm::VkFactory::createDevice(int index) -> box<IGPUDevice> {
    return nullptr;
}

#endif