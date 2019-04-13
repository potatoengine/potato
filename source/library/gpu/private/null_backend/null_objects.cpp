// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void up::gpu::null::FactoryNull::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto up::gpu::null::FactoryNull::createDevice(int index) -> rc<Device> {
    return new_shared<DeviceNull>();
}

UP_GPU_API auto up::gpu::CreateFactoryNull() -> box<Factory> {
    return new_box<null::FactoryNull>();
}

auto up::gpu::null::DeviceNull::createSwapChain(void* native_window) -> rc<SwapChain> {
    return new_shared<SwapChainNull>();
}

auto up::gpu::null::DeviceNull::createCommandList(PipelineState* pipelineState) -> box<CommandList> {
    return new_box<CommandListNull>();
}

auto up::gpu::null::DeviceNull::createPipelineState(PipelineStateDesc const&) -> box<PipelineState> {
    return new_box<PipelineStateNull>();
}

auto up::gpu::null::DeviceNull::createRenderTargetView(Texture* renderTarget) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::RTV);
}

auto up::gpu::null::DeviceNull::createDepthStencilView(Texture* depthStencilBuffer) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::DSV);
}

auto up::gpu::null::DeviceNull::createShaderResourceView(GpuBuffer* resource) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::SRV);
}

auto up::gpu::null::DeviceNull::createShaderResourceView(Texture* resource) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::SRV);
}

auto up::gpu::null::DeviceNull::createBuffer(BufferType type, up::uint64 size) -> box<GpuBuffer> {
    return new_box<BufferNull>(type);
}

auto up::gpu::null::DeviceNull::createTexture2D(TextureDesc const& desc, span<up::byte const> data) -> box<Texture> {
    return new_box<TextureNull>();
}

auto up::gpu::null::DeviceNull::createSampler() -> box<Sampler> {
    return new_box<SamplerNull>();
}

auto up::gpu::null::SwapChainNull::getBuffer(int index) -> box<Texture> {
    return new_box<TextureNull>();
}

int up::gpu::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
