// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "grimm/gpu/com_ptr.h"
#include "d3d11_device.h"
#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_buffer.h"
#include "d3d11_resource_view.h"
#include "d3d11_swap_chain.h"
#include "d3d11_platform.h"
#include "d3d11_texture.h"
#include "d3d11_sampler.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"
#include <utility>

gm::gpu::d3d11::DeviceD3D11::DeviceD3D11(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> context)
    : _factory(std::move(factory)), _adaptor(std::move(adapter)), _device(std::move(device)), _context(std::move(context)) {
    GM_ASSERT(_factory != nullptr);
    GM_ASSERT(_adaptor != nullptr);
    GM_ASSERT(_device != nullptr);
    GM_ASSERT(_context != nullptr);
}

gm::gpu::d3d11::DeviceD3D11::~DeviceD3D11() {
    _context.reset();

    com_ptr<ID3D11Debug> debug;
    _device->QueryInterface(__uuidof(ID3D11Debug), out_ptr(debug));

    _device.reset();
    _adaptor.reset();
    _factory.reset();

    if (debug != nullptr) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
    }
}

auto gm::gpu::d3d11::DeviceD3D11::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter) -> rc<Device> {
    GM_ASSERT(factory != nullptr);
    GM_ASSERT(adapter != nullptr);

    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

    com_ptr<ID3D11Device> device;
    com_ptr<ID3D11DeviceContext> context;
    D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG, levels, 1, D3D11_SDK_VERSION, out_ptr(device), nullptr, out_ptr(context));
    if (device == nullptr || context == nullptr) {
        return nullptr;
    }

    return new_shared<DeviceD3D11>(std::move(factory), std::move(adapter), std::move(device), std::move(context));
}

auto gm::gpu::d3d11::DeviceD3D11::createSwapChain(void* nativeWindow) -> rc<SwapChain> {
    GM_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D11::createSwapChain(_factory.get(), _device.get(), nativeWindow);
}

auto gm::gpu::d3d11::DeviceD3D11::createCommandList(PipelineState* pipelineState) -> box<CommandList> {
    return CommandListD3D11::createCommandList(_device.get(), pipelineState);
}

auto gm::gpu::d3d11::DeviceD3D11::createRenderTargetView(Texture* renderTarget) -> box<ResourceView> {
    GM_ASSERT(renderTarget != nullptr);

    auto d3d11Resource = static_cast<TextureD3D11*>(renderTarget);

    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    desc.Format = toNative(d3d11Resource->format());

    com_ptr<ID3D11RenderTargetView> view;
    HRESULT hr = _device->CreateRenderTargetView(d3d11Resource->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(ViewType::RTV, view.as<ID3D11View>());
}

auto gm::gpu::d3d11::DeviceD3D11::createDepthStencilView(Texture* depthStencilBuffer) -> box<ResourceView> {
    GM_ASSERT(depthStencilBuffer != nullptr);

    auto d3d11Resource = static_cast<TextureD3D11*>(depthStencilBuffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    desc.Format = toNative(d3d11Resource->format());

    com_ptr<ID3D11DepthStencilView> view;
    HRESULT hr = _device->CreateDepthStencilView(d3d11Resource->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(ViewType::DSV, view.as<ID3D11View>());
}

auto gm::gpu::d3d11::DeviceD3D11::createShaderResourceView(Buffer* resource) -> box<ResourceView> {
    GM_ASSERT(resource != nullptr);

    auto buffer = static_cast<BufferD3D11*>(resource);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.Buffer.NumElements = 0;
    desc.Buffer.ElementWidth = static_cast<UINT32>(buffer->size()) / (sizeof(float) * 4);

    com_ptr<ID3D11ShaderResourceView> view;
    HRESULT hr = _device->CreateShaderResourceView(buffer->buffer().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(ViewType::SRV, view.as<ID3D11View>());
}

auto gm::gpu::d3d11::DeviceD3D11::createShaderResourceView(Texture* texture) -> box<ResourceView> {
    GM_ASSERT(texture != nullptr);

    auto d3dTexture = static_cast<TextureD3D11*>(texture);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = toNative(texture->format());
    switch (texture->type()) {
    case TextureType::Texture2D:
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;
        break;
    }

    com_ptr<ID3D11ShaderResourceView> view;
    HRESULT hr = _device->CreateShaderResourceView(d3dTexture->get().get(), &desc, out_ptr(view));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<ResourceViewD3D11>(ViewType::SRV, view.as<ID3D11View>());
}

auto gm::gpu::d3d11::DeviceD3D11::createPipelineState(PipelineStateDesc const& desc) -> box<PipelineState> {
    return PipelineStateD3D11::createGraphicsPipelineState(desc, _device.get());
}

auto gm::gpu::d3d11::DeviceD3D11::createBuffer(BufferType type, gm::uint64 size) -> box<Buffer> {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = static_cast<UINT>(size);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    switch (type) {
    case BufferType::Index: desc.BindFlags = D3D11_BIND_INDEX_BUFFER; break;
    case BufferType::Vertex: desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; break;
    case BufferType::Constant: desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
    default: desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; break;
    }

    com_ptr<ID3D11Buffer> buffer;
    HRESULT hr = _device->CreateBuffer(&desc, nullptr, out_ptr(buffer));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<BufferD3D11>(type, size, std::move(buffer));
}

auto gm::gpu::d3d11::DeviceD3D11::createTexture2D(TextureDesc const& desc, span<gm::byte const> data) -> box<Texture> {
    auto bytesPerPixel = toByteSize(desc.format);

    GM_ASSERT(data.empty() || data.size() == desc.width * desc.height * bytesPerPixel);

    D3D11_TEXTURE2D_DESC nativeDesc = {};
    nativeDesc.Format = toNative(desc.format);
    nativeDesc.Width = desc.width;
    nativeDesc.Height = desc.height;
    nativeDesc.MipLevels = 1;
    nativeDesc.ArraySize = 1;
    nativeDesc.CPUAccessFlags = 0;
    if (desc.type == TextureType::DepthStencil) {
        nativeDesc.Usage = D3D11_USAGE_DEFAULT;
        nativeDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    }
    else {
        nativeDesc.Usage = D3D11_USAGE_IMMUTABLE;
        nativeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    }
    nativeDesc.SampleDesc.Count = 1;
    nativeDesc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = data.data();
    init.SysMemPitch = desc.width * bytesPerPixel;

    com_ptr<ID3D11Texture2D> texture;
    HRESULT hr = _device->CreateTexture2D(&nativeDesc, data.empty() ? nullptr : &init, out_ptr(texture));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<TextureD3D11>(std::move(texture).as<ID3D11Resource>());
}

auto gm::gpu::d3d11::DeviceD3D11::createSampler() -> box<Sampler> {
    D3D11_SAMPLER_DESC desc = {};
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.MaxAnisotropy = 1;
    desc.MaxLOD = 0;
    desc.MinLOD = 0;

    com_ptr<ID3D11SamplerState> sampler;
    HRESULT hr = _device->CreateSamplerState(&desc, out_ptr(sampler));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return new_box<SamplerD3D11>(std::move(sampler));
}

void gm::gpu::d3d11::DeviceD3D11::execute(CommandList* commandList) {
    GM_ASSERT(commandList != nullptr);

    auto deferred = static_cast<CommandListD3D11*>(commandList);

    GM_ASSERT(deferred->commandList(), "Command list is still open");

    _context->ExecuteCommandList(deferred->commandList().get(), FALSE);
}
