// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_VULKAN

#    include "vkn_factory.h"
#    include "device.h"
#    include "vkn_device.h"

gm::VknFactory::~VknFactory() = default;

auto gm::CreateVulkanGPUFactory() -> box<IGPUFactory> {
    return make_box<VknFactory>();
}

bool gm::VknFactory::isEnabled() const {
    return false;
}

void gm::VknFactory::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
}

auto gm::VknFactory::createDevice(int index) -> box<IGPUDevice> {
    return VknDevice::createDevice();
}

#endif
