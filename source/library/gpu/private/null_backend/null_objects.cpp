// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_objects.h"

void up::null::FactoryNull::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    static GpuDeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto up::null::FactoryNull::createDevice(int index) -> rc<GpuDevice> {
    return new_shared<DeviceNull>();
}

UP_GPU_API auto up::CreateFactoryNull() -> box<GpuDeviceFactory> {
    return new_box<null::FactoryNull>();
}

auto up::null::DeviceNull::createSwapChain(void* native_window) -> rc<GpuSwapChain> {
    return new_shared<SwapChainNull>();
}

auto up::null::DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return new_box<CommandListNull>();
}

auto up::null::DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return new_box<PipelineStateNull>();
}

auto up::null::DeviceNull::createRenderTargetView(GpuTexture* renderTarget) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::RTV);
}

auto up::null::DeviceNull::createDepthStencilView(GpuTexture* depthStencilBuffer) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::DSV);
}

auto up::null::DeviceNull::createShaderResourceView(GpuBuffer* resource) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::SRV);
}

auto up::null::DeviceNull::createShaderResourceView(GpuTexture* resource) -> box<GpuResourceView> {
    return new_box<ResourceViewNull>(GpuViewType::SRV);
}

auto up::null::DeviceNull::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    return new_box<BufferNull>(type);
}

auto up::null::DeviceNull::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> box<GpuTexture> {
    return new_box<TextureNull>();
}

auto up::null::DeviceNull::createSampler() -> box<GpuSampler> {
    return new_box<SamplerNull>();
}

auto up::null::SwapChainNull::getBuffer(int index) -> box<GpuTexture> {
    return new_box<TextureNull>();
}

int up::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
