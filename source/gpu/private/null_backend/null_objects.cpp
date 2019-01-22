// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::gpu::null::FactoryNull::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::gpu::null::FactoryNull::createDevice(int index) -> box<Device> {
    return make_box<DeviceNull>();
}

GM_GPU_API auto gm::gpu::CreateFactoryNull() -> box<Factory> {
    return make_box<null::FactoryNull>();
}

auto gm::gpu::null::DeviceNull::createSwapChain(void* native_window) -> box<GpuSwapChain> {
    return make_box<SwapChainNull>();
}

auto gm::gpu::null::DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> box<CommandList> {
    return make_box<CommandListNull>();
}

auto gm::gpu::null::DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return make_box<PipelineStateNull>();
}

auto gm::gpu::null::DeviceNull::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
    return make_box<ResourceViewNull>(ViewType::RTV);
}

auto gm::gpu::null::DeviceNull::createShaderResourceView(Buffer* resource) -> box<GpuResourceView> {
    return make_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::gpu::null::DeviceNull::createBuffer(BufferType type, gm::uint64 size) -> box<Buffer> {
    return make_box<BufferNull>(type);
}

auto gm::gpu::null::SwapChainNull::getBuffer(int index) -> box<GpuResource> {
    return make_box<ResourceNull>();
}

int gm::gpu::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
