// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::NullFactory::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::NullFactory::createDevice(int index) -> box<GpuDevice> {
    return make_box<NullDevice>();
}

GM_GPU_API auto gm::CreateNullGPUFactory() -> box<GpuDeviceFactory> {
    return make_box<NullFactory>();
}

auto gm::NullDevice::createSwapChain(void* native_window) -> box<GpuSwapChain> {
    return make_box<NullSwapChain>();
}

auto gm::NullDevice::createDescriptorHeap() -> box<GpuDescriptorHeap> {
    return make_box<NullDescriptorHeap>();
}

auto gm::NullDevice::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return make_box<NullCommandList>();
}

auto gm::NullDevice::createPipelineState() -> box<GpuPipelineState> {
    return make_box<NullPipelineState>();
}

auto gm::NullSwapChain::getBuffer(int index) -> box<GpuResource> {
    return make_box<NullResource>();
}

int gm::NullSwapChain::getCurrentBufferIndex() {
    return 0;
}

auto gm::NullDescriptorHeap::getCpuHandle() const -> GpuDescriptorHandle {
    return {0, sizeof(int)};
}
