// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void gm::gpu::null::FactoryNull::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    static DeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto gm::gpu::null::FactoryNull::createDevice(int index) -> rc<Device> {
    return make_shared<DeviceNull>();
}

GM_GPU_API auto gm::gpu::CreateFactoryNull() -> box<Factory> {
    return make_box<null::FactoryNull>();
}

auto gm::gpu::null::DeviceNull::createSwapChain(void* native_window) -> box<SwapChain> {
    return make_box<SwapChainNull>();
}

auto gm::gpu::null::DeviceNull::createCommandList(PipelineState* pipelineState) -> box<CommandList> {
    return make_box<CommandListNull>();
}

auto gm::gpu::null::DeviceNull::createPipelineState(PipelineStateDesc const&) -> box<PipelineState> {
    return make_box<PipelineStateNull>();
}

auto gm::gpu::null::DeviceNull::createRenderTargetView(Texture* renderTarget) -> box<ResourceView> {
    return make_box<ResourceViewNull>(ViewType::RTV);
}

auto gm::gpu::null::DeviceNull::createShaderResourceView(Buffer* resource) -> box<ResourceView> {
    return make_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::gpu::null::DeviceNull::createShaderResourceView(Texture* resource) -> box<ResourceView> {
    return make_box<ResourceViewNull>(ViewType::SRV);
}

auto gm::gpu::null::DeviceNull::createBuffer(BufferType type, gm::uint64 size) -> box<Buffer> {
    return make_box<BufferNull>(type);
}

auto gm::gpu::null::DeviceNull::createTexture2D(gm::uint32 width, gm::uint32 height, Format format, span<gm::byte const> data) -> box<Texture> {
    return make_box<TextureNull>();
}

auto gm::gpu::null::DeviceNull::createSampler() -> box<Sampler> {
    return make_box<SamplerNull>();
}

auto gm::gpu::null::SwapChainNull::getBuffer(int index) -> box<Texture> {
    return make_box<TextureNull>();
}

int gm::gpu::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
