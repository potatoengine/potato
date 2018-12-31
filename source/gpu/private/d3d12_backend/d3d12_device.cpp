// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_device.h"
#    include "com_ptr.h"
#    include "d3d12_command_list.h"
#    include "d3d12_descriptor_heap.h"
#    include "d3d12_pipeline_state.h"
#    include "d3d12_resource.h"
#    include "d3d12_swap_chain.h"
#    include "direct3d.h"
#    include "grimm/foundation/assert.h"
#    include "grimm/foundation/out_ptr.h"
#    include <utility>

gm::D3d12Device::D3d12Device(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D12Device1> device, com_ptr<ID3D12CommandQueue> graphicsQueue)
    : _factory(std::move(factory)), _adaptor(std::move(adapter)), _device(std::move(device)), _graphicsQueue(std::move(graphicsQueue)) {
    GM_ASSERT(_factory != nullptr);
    GM_ASSERT(_adaptor != nullptr);
    GM_ASSERT(_device != nullptr);
    GM_ASSERT(_graphicsQueue != nullptr);

    HRESULT hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), out_ptr(_executeFence));
    _fenceEvent = CreateEventW(nullptr, false, false, L"frame");
}

gm::D3d12Device::~D3d12Device() {
    _graphicsQueue.reset();
    _executeFence.reset();

    com_ptr<ID3D12DebugDevice> debug;
    _device->QueryInterface(__uuidof(ID3D12DebugDevice), out_ptr(debug));

    _device.reset();
    _adaptor.reset();
    _factory.reset();

    if (debug != nullptr) {
        debug->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
    }
}

auto gm::D3d12Device::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter) -> box<GpuDevice> {
    GM_ASSERT(factory != nullptr);
    GM_ASSERT(adapter != nullptr);

    // enable debug layer - MUST be done before creating device!
    com_ptr<ID3D12Debug> debugController0;
    D3D12GetDebugInterface(__uuidof(ID3D12Debug), out_ptr(debugController0));
    if (debugController0 != nullptr) {
        debugController0->EnableDebugLayer();

        // com_ptr<ID3D12Debug1> debugController1;
        // debugController0->QueryInterface(__uuidof(ID3D12Debug1), out_ptr(debugController1));
        // if (debugController1 != nullptr) {
        //     debugController1->SetEnableGPUBasedValidation(true);
        //     debugController1->SetEnableSynchronizedCommandQueueValidation(true);
        // }
    }

    com_ptr<ID3D12Device1> device;
    D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device1), out_ptr(device));
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    com_ptr<ID3D12CommandQueue> graphicsQueue;
    device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), out_ptr(graphicsQueue));
    if (graphicsQueue == nullptr) {
        return nullptr;
    }

    return make_box<D3d12Device>(std::move(factory), std::move(adapter), std::move(device), std::move(graphicsQueue));
}

auto gm::D3d12Device::createSwapChain(void* nativeWindow) -> box<GpuSwapChain> {
    GM_ASSERT(nativeWindow != nullptr);

    return D3d12SwapChain::createSwapChain(_factory.get(), _graphicsQueue.get(), nativeWindow);
}

auto gm::D3d12Device::createDescriptorHeap() -> box<GpuDescriptorHeap> {
    return D3d12DescriptorHeap::createDescriptorHeap(_device.get());
}

auto gm::D3d12Device::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return D3d12CommandList::createCommandList(_device.get(), pipelineState);
}

void gm::D3d12Device::createRenderTargetView(GpuResource* renderTarget, uint64 cpuHandle) {
    GM_ASSERT(renderTarget != nullptr);

    auto d3d12Resource = static_cast<D3d12Resource*>(renderTarget);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = {cpuHandle};
    _device->CreateRenderTargetView(d3d12Resource->get().get(), nullptr, handle);
}

auto gm::D3d12Device::createPipelineState() -> box<GpuPipelineState> {
    return D3d12PipelineState::createGraphicsPipelineState(_device.get());
}

void gm::D3d12Device::execute(GpuCommandList* commandList) {
    GM_ASSERT(commandList != nullptr);

    ID3D12GraphicsCommandList* graphicsList = static_cast<D3d12CommandList*>(commandList)->get().get();
    ID3D12CommandList* list = graphicsList;

    ResetEvent(_fenceEvent.get());
    _executeFence->Signal(0);
    _executeFence->SetEventOnCompletion(1, _fenceEvent.get());

    graphicsList->Close();
    _graphicsQueue->ExecuteCommandLists(1, &list);
    _graphicsQueue->Signal(_executeFence.get(), 1);

    WaitForSingleObject(_fenceEvent.get(), INFINITE);
}

#endif
