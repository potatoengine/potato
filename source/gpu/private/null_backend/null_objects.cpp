// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::gpu::null::FactoryNull::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::gpu::null::FactoryNull::createDevice(int index) -> rc<Device> {
    return new_shared<DeviceNull>();
}

GM_GPU_API auto gm::gpu::CreateFactoryNull() -> box<Factory> {
    return new_box<null::FactoryNull>();
}

auto gm::gpu::null::DeviceNull::createSwapChain(void* native_window) -> rc<SwapChain> {
    return new_shared<SwapChainNull>();
}

auto gm::gpu::null::DeviceNull::createCommandList(PipelineState* pipelineState) -> box<CommandList> {
    return new_box<CommandListNull>();
}

auto gm::gpu::null::DeviceNull::createPipelineState(PipelineStateDesc const&) -> box<PipelineState> {
    return new_box<PipelineStateNull>();
}

auto gm::gpu::null::DeviceNull::createRenderTargetView(Texture* renderTarget) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::RTV);
}

auto gm::gpu::null::DeviceNull::createDepthStencilView(Texture* depthStencilBuffer) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::DSV);
}

auto gm::gpu::null::DeviceNull::createShaderResourceView(Buffer* resource) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::gpu::null::DeviceNull::createShaderResourceView(Texture* resource) -> box<ResourceView> {
    return new_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::gpu::null::DeviceNull::createBuffer(BufferType type, gm::uint64 size) -> box<Buffer> {
    return new_box<BufferNull>(type);
}

auto gm::gpu::null::DeviceNull::createTexture2D(TextureDesc const& desc, span<gm::byte const> data) -> box<Texture> {
    return new_box<TextureNull>();
}

auto gm::gpu::null::DeviceNull::createSampler() -> box<Sampler> {
    return new_box<SamplerNull>();
}

auto gm::gpu::null::SwapChainNull::getBuffer(int index) -> box<Texture> {
    return new_box<TextureNull>();
}

int gm::gpu::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
