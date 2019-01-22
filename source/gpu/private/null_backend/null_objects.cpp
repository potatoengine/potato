// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::FactoryNull::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::FactoryNull::createDevice(int index) -> box<gpu::GpuDevice> {
    return make_box<DeviceNull>();
}

GM_GPU_API auto gm::gpu::CreateNullGPUFactory() -> box<gpu::GpuDeviceFactory> {
    return make_box<FactoryNull>();
}

auto gm::DeviceNull::createSwapChain(void* native_window) -> box<GpuSwapChain> {
    return make_box<SwapChainNull>();
}

auto gm::DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return make_box<CommandListNull>();
}

auto gm::DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return make_box<PipelineStateNull>();
}

auto gm::DeviceNull::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
    return make_box<ResourceViewNull>(ViewType::RTV);
}

auto gm::DeviceNull::createShaderResourceView(GpuBuffer* resource) -> box<GpuResourceView> {
    return make_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::DeviceNull::createBuffer(BufferType type, gm::uint64 size) -> box<GpuBuffer> {
    return make_box<BufferNull>(type);
}

auto gm::SwapChainNull::getBuffer(int index) -> box<GpuResource> {
    return make_box<ResourceNull>();
}

int gm::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
