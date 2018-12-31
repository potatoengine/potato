// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

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

auto gm::NullDevice::createSwapChain(void* native_window) -> box<ISwapChain> {
    return make_box<NullSwapChain>();
}

auto gm::NullDevice::createDescriptorHeap() -> box<IDescriptorHeap> {
    return make_box<NullDescriptorHeap>();
}

auto gm::NullDevice::createCommandList(IPipelineState* pipelineState) -> box<ICommandList> {
    return make_box<NullCommandList>();
}

auto gm::NullDevice::createPipelineState() -> box<IPipelineState> {
    return make_box<NullPipelineState>();
}

auto gm::NullSwapChain::getBuffer(int index) -> box<IGpuResource> {
    return make_box<NullResource>();
}

int gm::NullSwapChain::getCurrentBufferIndex() {
    return 0;
}

auto gm::NullDescriptorHeap::getCpuHandle() const -> DescriptorHandle {
    return {0, sizeof(int)};
}
