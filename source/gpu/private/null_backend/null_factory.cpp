// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_factory.h"
#include "null_device.h"

gm::NullFactory::NullFactory() = default;

gm::NullFactory::~NullFactory() = default;

bool gm::NullFactory::isEnabled() const { return true; }

void gm::NullFactory::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::NullFactory::createDevice(int index) -> box<IGPUDevice> {
    return make_box<NullDevice>();
}

GM_GPU_API auto gm::CreateNullGPUFactory() -> box<IGPUFactory> {
    return make_box<NullFactory>();
}
