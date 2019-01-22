// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::NullFactory::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    static GpuDeviceInfo deviceInfo = {0};

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

auto gm::NullDevice::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return make_box<NullCommandList>();
}

auto gm::NullDevice::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return make_box<NullPipelineState>();
}

auto gm::NullDevice::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
    return make_box<NullResourceView>(ViewType::RTV);
}

auto gm::NullDevice::createShaderResourceView(GpuBuffer* resource) -> box<GpuResourceView> {
    return make_box<NullResourceView>(ViewType::SRV);
}

auto gm::NullDevice::createBuffer(BufferType type, gm::uint64 size) -> box<GpuBuffer> {
    return make_box<NullBuffer>(type);
}

auto gm::NullSwapChain::getBuffer(int index) -> box<GpuResource> {
    return make_box<NullResource>();
}

int gm::NullSwapChain::getCurrentBufferIndex() {
    return 0;
}
