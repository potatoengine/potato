// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_device.h"
#include "com_ptr.h"
#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource.h"
#include "d3d11_swap_chain.h"
#include "d3d11_platform.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/gpu/descriptor_heap.h"
#include <utility>

gm::DeviceD3D11::DeviceD3D11(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> context)
    : _factory(std::move(factory)), _adaptor(std::move(adapter)), _device(std::move(device)), _context(std::move(context)) {
    GM_ASSERT(_factory != nullptr);
    GM_ASSERT(_adaptor != nullptr);
    GM_ASSERT(_device != nullptr);
    GM_ASSERT(_context != nullptr);
}

gm::DeviceD3D11::~DeviceD3D11() {
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

auto gm::DeviceD3D11::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter) -> box<GpuDevice> {
    GM_ASSERT(factory != nullptr);
    GM_ASSERT(adapter != nullptr);

    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

    com_ptr<ID3D11Device> device;
    com_ptr<ID3D11DeviceContext> context;
    D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE, levels, 1, D3D11_SDK_VERSION, out_ptr(device), nullptr, out_ptr(context));
    if (device == nullptr || context == nullptr) {
        return nullptr;
    }

    return make_box<DeviceD3D11>(std::move(factory), std::move(adapter), std::move(device), std::move(context));
}

auto gm::DeviceD3D11::createSwapChain(void* nativeWindow) -> box<GpuSwapChain> {
    GM_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D11::createSwapChain(_factory.get(), _device.get(), nativeWindow);
}

auto gm::DeviceD3D11::createDescriptorHeap() -> box<GpuDescriptorHeap> {
    return nullptr;
}

auto gm::DeviceD3D11::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return CommandListD3D11::createCommandList(_device.get(), pipelineState);
}

void gm::DeviceD3D11::createRenderTargetView(GpuResource* renderTarget, gm::uint64 cpuHandle) {
    GM_UNREACHABLE("unsupported");
}

auto gm::DeviceD3D11::createRenderTargetView(GpuResource* renderTarget) -> box<GpuResourceView> {
    GM_ASSERT(renderTarget != nullptr);

    auto d3d11Resource = static_cast<ResourceD3D11*>(renderTarget);

    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    com_ptr<ID3D11RenderTargetView> view;
    _device->CreateRenderTargetView(d3d11Resource->get().get(), &desc, out_ptr(view));
}

auto gm::DeviceD3D11::createPipelineState(GpuPipelineStateDesc const& desc) -> box<GpuPipelineState> {
    return PipelineStateD3D11::createGraphicsPipelineState(desc, _device.get());
}

void gm::DeviceD3D11::execute(GpuCommandList* commandList) {
    GM_ASSERT(commandList != nullptr);

    com_ptr<ID3D11DeviceContext> const& deferred = static_cast<CommandListD3D11*>(commandList)->getDeviceContext();
    com_ptr<ID3D11CommandList> commands;
    deferred->FinishCommandList(FALSE, out_ptr(commands));

    _context->ExecuteCommandList(commands.get(), TRUE);
}
