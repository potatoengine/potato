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
#include "d3d12_context.h"
#include "d3d12_renderable.h"
#include "d3d12_utils.h"

#include "context.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/com_ptr.h"
#include "potato/spud/out_ptr.h"

#include <utility>
#include <d3d12.h>
#include <D3D12MemAlloc.h>

up::d3d12::DeviceD3D12::DeviceD3D12(
    IDXGIFactoryPtr factory,
    IDXGIAdapterPtr adapter)
    : _factory(std::move(factory))
    , _adapter(std::move(adapter))
{ 
    UP_ASSERT(_factory != nullptr);
    UP_ASSERT(_adapter != nullptr);
}

up::d3d12::DeviceD3D12::~DeviceD3D12() {
    _rtvHeap.reset();
    _srvUavHeap.reset();
    _allocator->Release();
    _allocator = nullptr; 
    _device.reset();
    _adapter.reset();
    _factory.reset();

}

auto up::d3d12::DeviceD3D12::createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter) -> rc<GpuDevice> {
    UP_ASSERT(factory != nullptr);
    UP_ASSERT(adapter != nullptr);

    auto device = new_shared<DeviceD3D12>(std::move(factory), std::move(adapter));
    device->create();
    return std::move(device);
}

bool up::d3d12::DeviceD3D12::create() {

    D3D12CreateDevice(_adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), out_ptr(_device));
    if (_device == nullptr) {
        return false;
    }

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    _device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), out_ptr(_rtvHeap));

    // Describe and create a shader resource view (SRV) and unordered
    // access view (UAV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};
    srvUavHeapDesc.NumDescriptors = 1; 
    srvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    _device->CreateDescriptorHeap(&srvUavHeapDesc, __uuidof(ID3D12DescriptorHeap), out_ptr(_srvUavHeap));

    _rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _srvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    _device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), out_ptr(_commandQueue));
    SetName(_commandQueue.get(), L"commnadQueue");

    createAllocator();

    return true;
}

auto up::d3d12::DeviceD3D12::createRenderable(IRenderable* pInterface) -> box<GpuRenderable> {
    // @todo: use smarter allocation routine
    return new_box<RenderableD3D12>(pInterface);
}

auto up::d3d12::DeviceD3D12::createSwapChain(void* nativeWindow) -> rc<GpuSwapChain> {
    UP_ASSERT(nativeWindow != nullptr);

    return SwapChainD3D12::createSwapChain(_factory.get(), _device.get(), nativeWindow);
}

auto up::d3d12::DeviceD3D12::createCommandList(GpuPipelineState* pipelineState) -> box<GpuCommandList> {
    return CommandListD3D12::createCommandList(_device.get(), pipelineState);
}

auto up::d3d12::DeviceD3D12::createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* swapChain) -> rc<GpuTexture> {
    UP_ASSERT(swapChain != nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvHeap->GetCPUDescriptorHandleForHeapStart());

  /*  auto rt = static_cast<SwapChainD3D12*>(swapChain)->getBuffer(0);
    _device->CreateRenderTargetView(rt.get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, _rtvDescriptorSize);*/

    return new_shared<TextureD3D12>();
}
auto up::d3d12::DeviceD3D12::createPipelineState(GpuPipelineStateDesc const& desc) -> box<GpuPipelineState> {
    return PipelineStateD3D12::createGraphicsPipelineState(desc, _device.get());
}

auto up::d3d12::DeviceD3D12::createBuffer(GpuBufferType type, up::uint64 size) -> box<GpuBuffer> {
    

    return new_box<BufferD3D12>(type, size, nullptr);
}

auto up::d3d12::DeviceD3D12::createTexture2D(GpuTextureDesc const& desc, span<up::byte const> data) -> rc<GpuTexture> {
    auto bytesPerPixel = toByteSize(desc.format);

    UP_ASSERT(data.empty() || data.size() == desc.width * desc.height * bytesPerPixel);
    auto texture = new_shared<TextureD3D12>();

    ContextD3D12 ctx = {_device.get(), _commandList.get(), _allocator};
    texture->create(ctx, desc, data);
    return texture;
}

auto up::d3d12::DeviceD3D12::createSampler() -> box<GpuSampler> {
   
    return new_box<SamplerD3D12>();
}

void up::d3d12::DeviceD3D12::createAllocator() {
    static constexpr D3D12MA::ALLOCATOR_FLAGS allocatorFlags = D3D12MA::ALLOCATOR_FLAG_NONE;

    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = allocatorFlags;
    desc.pDevice = _device.get();
    desc.PreferredBlockSize = 0;

    // if (ENABLE_CPU_ALLOCATION_CALLBACKS) {
    //    g_AllocationCallbacks.pAllocate = &CustomAllocate;
    //    g_AllocationCallbacks.pFree = &CustomFree;
    //    g_AllocationCallbacks.pUserData = CUSTOM_ALLOCATION_USER_DATA;
    //    desc.pAllocationCallbacks = &g_AllocationCallbacks;
    //}

    HRESULT hr = D3D12MA::CreateAllocator(&desc, &_allocator);

    switch (_allocator->GetD3D12Options().ResourceHeapTier) {
        case D3D12_RESOURCE_HEAP_TIER_1:
            wprintf(L"ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1\n");
            break;
        case D3D12_RESOURCE_HEAP_TIER_2:
            wprintf(L"ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_2\n");
            break;
        default:
            assert(0);
    }
}

void up::d3d12::DeviceD3D12::createFrameSync() {

    _device->CreateFence(_renderContextFenceValue, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), out_ptr(_renderFrameFence));
    _renderContextFenceValue++;

    _renderContextFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (_renderContextFenceEvent == nullptr) {
        // @todo -- add fatal error handling 
        return; 
    }

    waitForFrame();
}

void up::d3d12::DeviceD3D12::waitForFrame() {

    // Add a signal command to the queue.
    _commandQueue->Signal(_renderFrameFence.get(), _renderContextFenceValue);

    // Instruct the fence to set the event object when the signal command completes.
    _renderFrameFence->SetEventOnCompletion(_renderContextFenceValue, _renderContextFenceEvent);
    _renderContextFenceValue++;

    // Wait until the signal command has been processed.
    WaitForSingleObject(_renderContextFenceEvent, INFINITE);
}
void up::d3d12::DeviceD3D12::render(const FrameData& frameData, GpuRenderable* renderable) {
    UP_ASSERT(renderable);
    RenderContext ctx = {frameData.lastFrameTimeDelta, *_commandList.get(), *this};
    static_cast<RenderableD3D12*>(renderable)->onRender(ctx);
}

void up::d3d12::DeviceD3D12::execute() {
    UP_ASSERT(_commandQueue != nullptr);

    waitForFrame();

    // Close the command list and execute it to begin the initial GPU setup.
    _commandList->flush(_commandQueue.get());

 /*   auto deferred = static_cast<CommandListD3D12*>(commandList);

    UP_ASSERT(deferred->commandList(), "Command list is still open");

    _context->ExecuteCommandList(deferred->commandList().get(), FALSE);*/
}
