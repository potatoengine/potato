// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "null_objects.h"

void up::null::FactoryNull::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    static GpuDeviceInfo deviceInfo = {0};

    callback(deviceInfo);
}

auto up::null::FactoryNull::createDevice(int index) -> rc<GpuDevice> {
    return new_shared<DeviceNull>();
}

auto up::CreateFactoryNull() -> box<GpuDeviceFactory> {
    return new_box<null::FactoryNull>();
}

auto up::null::DeviceNull::createRenderable(IRenderable* pInterface) -> box<GpuRenderable>{
    return new_box<RenderableNull>();
}

auto up::null::DeviceNull::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    return new_shared<SwapChainNull>();
}

auto up::null::DeviceNull::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return new_box<CommandListNull>();
}

auto up::null::DeviceNull::createPipelineState(GpuPipelineStateDesc const&) -> box<GpuPipelineState> {
    return new_box<PipelineStateNull>();
}

auto up::null::DeviceNull::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    return new_box<BufferNull>(type);
}

auto up::null::DeviceNull::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    return new_shared<TextureNull>();
}

auto up::null::DeviceNull::createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* swapChain) -> rc<GpuTexture> {
    return new_shared<TextureNull>();
}

auto up::null::DeviceNull::createSampler() -> box<GpuSampler> {
    return new_box<SamplerNull>();
}

auto up::null::SwapChainNull::getBuffer(int index) -> rc<GpuTexture> {
    return new_shared<TextureNull>();
}

int up::null::SwapChainNull::getCurrentBufferIndex() {
    return 0;
}
