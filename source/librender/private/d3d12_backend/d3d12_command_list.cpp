// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_command_list.h"
#include "d3d12_buffer.h"
#include "d3d12_pipeline_state.h"
#include "d3d12_platform.h"
#include "d3d12_resource_view.h"
#include "d3d12_sampler.h"
#include "d3d12_texture.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/int_types.h"
#include "potato/spud/out_ptr.h"

up::d3d12::CommandListD3D12::CommandListD3D12() {}

up::d3d12::CommandListD3D12::~CommandListD3D12() = default;

auto up::d3d12::CommandListD3D12::createCommandList(ID3D12Device* device, GpuPipelineState* pipelineState)
    -> box<CommandListD3D12> {

    auto cl = new_box<CommandListD3D12>();
    cl->create(device, pipelineState); 
    return std::move(cl);
}

auto up::d3d12::CommandListD3D12::create(ID3D12Device * device, GpuPipelineState * pipelineState)
        -> bool {
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), out_ptr(_commandAllocator));

    auto d3dPipelineState = static_cast<PipelineStateD3D12*>(pipelineState);
    HRESULT hr = device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _commandAllocator.get(),
        d3dPipelineState->getState().get(),
        __uuidof(ID3D12GraphicsCommandList),
        out_ptr(_commandList));
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void up::d3d12::CommandListD3D12::setPipelineState(GpuPipelineState* state) {
    UP_ASSERT(state != nullptr);
}
  

void up::d3d12::CommandListD3D12::bindRenderTarget(up::uint32 index, GpuResourceView* view) {
    UP_ASSERT(index < maxRenderTargetBindings);

 }

void up::d3d12::CommandListD3D12::bindDepthStencil(GpuResourceView* view) {
    auto dsv = static_cast<ResourceViewD3D12*>(view);
    UP_ASSERT(dsv->type() == GpuViewType::DSV);

 }

void up::d3d12::CommandListD3D12::bindIndexBuffer(GpuBuffer* buffer, GpuIndexFormat indexType, up::uint32 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Index);

 }

void up::d3d12::CommandListD3D12::bindVertexBuffer(
    up::uint32 slot,
    GpuBuffer* buffer,
    up::uint64 stride,
    up::uint64 offset) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Vertex);
}

void up::d3d12::CommandListD3D12::bindConstantBuffer(up::uint32 slot, GpuBuffer* buffer, GpuShaderStage stage) {
    UP_ASSERT(buffer != nullptr);
    UP_ASSERT(buffer->type() == GpuBufferType::Constant);

}

void up::d3d12::CommandListD3D12::bindShaderResource(up::uint32 slot, GpuResourceView* view, GpuShaderStage stage) {
    UP_ASSERT(view != nullptr);

}

void up::d3d12::CommandListD3D12::bindSampler(up::uint32 slot, GpuSampler* sampler, GpuShaderStage stage) {
    UP_ASSERT(sampler != nullptr);

}

void up::d3d12::CommandListD3D12::setPrimitiveTopology(GpuPrimitiveTopology topology) {
    D3D12_PRIMITIVE_TOPOLOGY primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    switch (topology) {
        case GpuPrimitiveTopology::Lines:
            primitive = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
    }
    _commandList->IASetPrimitiveTopology(primitive);
}

void up::d3d12::CommandListD3D12::setViewport(GpuViewportDesc const& viewport) {
    D3D12_VIEWPORT d3d12Viewport;
    d3d12Viewport.TopLeftX = viewport.leftX;
    d3d12Viewport.TopLeftY = viewport.topY;
    d3d12Viewport.Width = viewport.width;
    d3d12Viewport.Height = viewport.height;
    d3d12Viewport.MinDepth = viewport.minDepth;
    d3d12Viewport.MaxDepth = viewport.maxDepth;
    _commandList->RSSetViewports(1, &d3d12Viewport);
}

void up::d3d12::CommandListD3D12::setClipRect(GpuClipRect rect) {
    D3D12_RECT clipRect;
    clipRect.left = static_cast<LONG>(rect.left);
    clipRect.top = static_cast<LONG>(rect.top);
    clipRect.right = static_cast<LONG>(rect.right);
    clipRect.bottom = static_cast<LONG>(rect.bottom);
    _commandList->RSSetScissorRects(1, &clipRect);
}

void up::d3d12::CommandListD3D12::draw(up::uint32 vertexCount, up::uint32 firstVertex) {
    _flushBindings();

}

void up::d3d12::CommandListD3D12::drawIndexed(up::uint32 indexCount, up::uint32 firstIndex, up::uint32 baseIndex) {
    _flushBindings();

}

void up::d3d12::CommandListD3D12::clearRenderTarget(GpuResourceView* view, glm::vec4 color) {
    UP_ASSERT(view != nullptr);

    FLOAT c[4] = {color.x, color.y, color.z, color.w};
}

void up::d3d12::CommandListD3D12::clearDepthStencil(GpuResourceView* view) {
    UP_ASSERT(view != nullptr);
}

void up::d3d12::CommandListD3D12::finish() {
}

void up::d3d12::CommandListD3D12::clear(GpuPipelineState* pipelineState) {

    auto d3dPipelinState = static_cast<PipelineStateD3D12*>(pipelineState); 
    _commandList->ClearState(d3dPipelinState->getState().get());
    _commandList->RSSetScissorRects(0, nullptr);
    if (pipelineState != nullptr) {
        setPipelineState(pipelineState);
    }
}

void up::d3d12::CommandListD3D12::flush(ID3D12CommandQueue* queue) {

    UP_ASSERT(queue);

    // done with this command list -- close it before we submit it.
    _commandList->Close();

    ID3D12CommandList* ppCommandLists[] = {_commandList.get()};
    queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}


auto up::d3d12::CommandListD3D12::map(GpuBuffer* buffer, up::uint64 size, up::uint64 offset) -> span<up::byte> {
    if (buffer == nullptr) {
        return {};
    }

    return {static_cast<up::byte*>(nullptr), static_cast<std::size_t>(size)};
}

void up::d3d12::CommandListD3D12::unmap(GpuBuffer* buffer, span<up::byte const> data) {
    if (buffer == nullptr) {
        return;
    }
}

void up::d3d12::CommandListD3D12::update(GpuBuffer* buffer, span<up::byte const> data, up::uint64 offset) {
    if (buffer == nullptr) {
        return;
    }

}

void up::d3d12::CommandListD3D12::_flushBindings() {
  
}
