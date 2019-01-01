// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "vkn_factory.h"
#include "grimm/gpu/device.h"
#include "vkn_device.h"

gm::VknFactory::~VknFactory() = default;

#if GM_GPU_ENABLE_VULKAN
auto gm::CreateVulkanGPUFactory() -> box<GpuDeviceFactory> {
    return make_box<VknFactory>();
}
#endif

bool gm::VknFactory::isEnabled() const {
    return false;
}

void gm::VknFactory::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
}

auto gm::VknFactory::createDevice(int index) -> box<GpuDevice> {
    return VknDevice::createDevice();
}
