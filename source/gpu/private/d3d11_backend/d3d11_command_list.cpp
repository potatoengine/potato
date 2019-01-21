// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource.h"
#include "d3d11_resource_view.h"
#include "d3d11_buffer.h"
#include "d3d11_platform.h"
#include "grimm/foundation/types.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"

gm::CommandListD3D11::CommandListD3D11(com_ptr<ID3D11DeviceContext> context) : _context(std::move(context)) {}

gm::CommandListD3D11::~CommandListD3D11() = default;

auto gm::CommandListD3D11::createCommandList(ID3D11Device* device, GpuPipelineState* pipelineState) -> box<CommandListD3D11> {
    com_ptr<ID3D11DeviceContext> context;
    HRESULT hr = device->CreateDeferredContext(0, out_ptr(context));
    if (context == nullptr) {
        return nullptr;
    }

    return make_box<CommandListD3D11>(std::move(context));
}

void gm::CommandListD3D11::setPipelineState(GpuPipelineState* state) {
    GM_ASSERT(state != nullptr);

    auto pipelineState = static_cast<PipelineStateD3D11*>(state);

    _context->IASetInputLayout(pipelineState->inputLayout().get());
    _context->RSSetState(pipelineState->rasterState().get());
    _context->OMSetBlendState(pipelineState->blendState().get(), nullptr, ~UINT(0));
    _context->OMSetDepthStencilState(pipelineState->depthStencilState().get(), 0);
}

void gm::CommandListD3D11::bindRenderTarget(gm::uint32 index, GpuResourceView* view) {
    GM_ASSERT(index < maxRenderTargetBindings);

    if (view == nullptr) {
        _rtv[index].reset();
        return;
    }

    auto rtv = static_cast<ResourceViewD3D11*>(view);
    GM_ASSERT(rtv->type() == ViewType::RTV);

    _rtv[index] = rtv->getView().as<ID3D11RenderTargetView>();
}

void gm::CommandListD3D11::bindBuffer(gm::uint32 slot, GpuResourceView* view) {
}

void gm::CommandListD3D11::clearRenderTarget(GpuResourceView* view, PackedVector4f color) {
    GM_ASSERT(view != nullptr);

    _context->ClearRenderTargetView(static_cast<ID3D11RenderTargetView*>(static_cast<ResourceViewD3D11*>(view)->getView().get()), color);
}

void gm::CommandListD3D11::finish() {
    _context->OMSetRenderTargets(maxRenderTargetBindings, reinterpret_cast<ID3D11RenderTargetView**>(&_rtv), _dsv.get());
    _context->FinishCommandList(FALSE, out_ptr(_commands));
}

void gm::CommandListD3D11::clear(GpuPipelineState* pipelineState) {
    _context->ClearState();
    _commands.reset();
    if (pipelineState != nullptr) {
        setPipelineState(pipelineState);
    }
}

auto gm::CommandListD3D11::map(GpuBuffer* resource, gm::uint64 size, gm::uint64 offset) -> span<gm::byte> {
    if (resource == nullptr) {
        return {};
    }

    auto buffer = static_cast<BufferD3D11*>(resource);
    ID3D11Buffer* d3dBuffer = buffer->buffer().get();
    GM_ASSERT(d3dBuffer != nullptr);

    D3D11_MAPPED_SUBRESOURCE sub = {};

    GM_ASSERT(offset < buffer->size());
    GM_ASSERT(size <= buffer->size() - offset);

    bool writeAll = offset == 0 && size == buffer->size();

    _context->Map(d3dBuffer, 0, writeAll ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &sub);
    return {static_cast<gm::byte*>(sub.pData) + offset, size};
}

void gm::CommandListD3D11::unmap(GpuBuffer* buffer, span<gm::byte const> data) {
    if (buffer == nullptr) {
        return;
    }

    ID3D11Buffer* d3dBuffer = static_cast<BufferD3D11*>(buffer)->buffer().get();

    _context->Unmap(d3dBuffer, 0);
}

void gm::CommandListD3D11::update(GpuBuffer* buffer, span<gm::byte const> data, gm::uint64 offset) {
    if (buffer == nullptr) {
        return;
    }

    auto target = map(buffer, data.size(), offset);
    std::memcpy(target.data(), data.data(), data.size());
    unmap(buffer, target);
}
