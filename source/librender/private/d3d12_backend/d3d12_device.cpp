// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_device.h"
#include "d3d12_buffer.h"
#include "d3d12_command_list.h"
#include "d3d12_pipeline_state.h"
#include "d3d12_platform.h"
#include "d3d12_resource_view.h"
#include "d3d12_sampler.h"
#include "d3d12_swap_chain.h"
#include "d3d12_texture.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <utility>

up::d3d12::DeviceD3D12::DeviceD3D12(
    IDXGIFactoryPtr factory,
    IDXGIAdapterPtr adapter,
    ID3DDevicePtr device
    : _factory(std::move(factory))
    , _adaptor(std::move(adapter))
    , _device(std::move(device)) {
    UP_ASSERT(_factory != nullptr);
    UP_ASSERT(_adaptor != nullptr);
    UP_ASSERT(_device != nullptr);
}

up::d3d12::DeviceD3D12::~DeviceD3D12() {
    _context.reset();

    com_ptr<ID3D12Debug> debug;
    _device->QueryInterface(__uuidof(ID3D12Debug), out_ptr(debug));

    _device.reset();
    _adaptor.reset();
    _factory.reset();

    if (debug != nullptr) {
        debug->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
}

auto up::d3d12::DeviceD3D12::createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter)
    -> rc<GpuDevice> {
    UP_ASSERT(factory != nullptr);
    UP_ASSERT(adapter != nullptr);

    com_ptr<ID3D12Device> device;
    D3D12CreateDevice(
        adapter.get(),
        D3D_FEATURE_LEVEL_11_0,
        out_ptr(device));
    if (device == nullptr || context == nullptr) {
        return nullptr;
    }

    return new_shared<DeviceD3D12>(std::move(factory), std::move(adapter), std::move(device));
}

auto up::d3d12::DeviceD3D12::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    UP_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D12::createSwapChain(_factory.get(), _device.get(), nativeWindow);
}

auto up::d3d12::DeviceD3D12::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return CommandListD3D12::createCommandList(_device.get(), pipelineState);
}

auto up::d3d12::DeviceD3D12::createRenderTargetView(GpuTexture* renderTarget) -> box<GpuResourceView> {
    UP_ASSERT(renderTarget != nullptr);

    auto d3d12Resource = static_cast<TextureD3D12*>(renderTarget);


    return new_box<ResourceViewD3D12>(GpuViewType::RTV, view.as<ID3D12View>());
}

auto up::d3d12::DeviceD3D12::createDepthStencilView(GpuTexture* depthStencilBuffer) -> box<GpuResourceView> {
    UP_ASSERT(depthStencilBuffer != nullptr);
   

    return new_box<ResourceViewD3D12>(GpuViewType::DSV, view.as<ID3D12View>());
}

auto up::d3d12::DeviceD3D12::createShaderResourceView(GpuBuffer* resource) -> box<GpuResourceView> {
    UP_ASSERT(resource != nullptr);

    auto buffer = static_cast<BufferD3D12*>(resource);

    return new_box<ResourceViewD3D12>(GpuViewType::SRV, view.as<ID3D12View>());
}

auto up::d3d12::DeviceD3D12::createShaderResourceView(GpuTexture* texture) -> box<GpuResourceView> {
    UP_ASSERT(texture != nullptr);

   

    return new_box<ResourceViewD3D12>(GpuViewType::SRV, view.as<ID3D12View>());
}

auto up::d3d12::DeviceD3D12::createPipelineState(GpuPipelineStateDesc const& desc) -> box<GpuPipelineState> {
    return PipelineStateD3D12::createGraphicsPipelineState(desc, _device.get());
}

auto up::d3d12::DeviceD3D12::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    

    return new_box<BufferD3D12>(type, size, std::move(buffer));
}

auto up::d3d12::DeviceD3D12::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    auto bytesPerPixel = toByteSize(desc.format);

    UP_ASSERT(data.empty() || data.size() == desc.width * desc.height * bytesPerPixel);

    D3D12_TEXTURE2D_DESC nativeDesc = {};
    nativeDesc.Format = toNative(desc.format);
    nativeDesc.Width = desc.width;
    nativeDesc.Height = desc.height;
    nativeDesc.MipLevels = 1;
    nativeDesc.ArraySize = 1;
    nativeDesc.CPUAccessFlags = 0;
    if (desc.type == GpuTextureType::DepthStencil) {
        nativeDesc.Usage = D3D12_USAGE_DEFAULT;
        nativeDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    }
    else {
        nativeDesc.Usage = D3D12_USAGE_DEFAULT;
        nativeDesc.BindFlags = D3D12_BIND_SHADER_RESOURCE | D3D12_BIND_RENDER_TARGET;
    }
    nativeDesc.SampleDesc.Count = 1;
    nativeDesc.SampleDesc.Quality = 0;

    D3D12_SUBRESOURCE_DATA init = {};
    init.pSysMem = data.data();
    init.SysMemPitch = desc.width * bytesPerPixel;

    com_ptr<ID3D12Texture2D> texture;
    HRESULT hr = _device->CreateTexture2D(&nativeDesc, data.empty() ? nullptr : &init, out_ptr(texture));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_shared<TextureD3D12>(std::move(texture).as<ID3D12Resource>());
}

auto up::d3d12::DeviceD3D12::createSampler() -> box<GpuSampler> {
   
    return new_box<SamplerD3D12>(std::move(sampler));
}

void up::d3d12::DeviceD3D12::execute(GpuCommandList* commandList) {
    UP_ASSERT(commandList != nullptr);

    auto deferred = static_cast<CommandListD3D12*>(commandList);

    UP_ASSERT(deferred->commandList(), "Command list is still open");

    _context->ExecuteCommandList(deferred->commandList().get(), FALSE);
}
