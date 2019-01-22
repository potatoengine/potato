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

gm::gpu::d3d11::CommandListD3D11::CommandListD3D11(com_ptr<ID3D11DeviceContext> context) : _context(std::move(context)) {}

gm::gpu::d3d11::CommandListD3D11::~CommandListD3D11() = default;

auto gm::gpu::d3d11::CommandListD3D11::createCommandList(ID3D11Device* device, GpuPipelineState* pipelineState) -> box<CommandListD3D11> {
    com_ptr<ID3D11DeviceContext> context;
    HRESULT hr = device->CreateDeferredContext(0, out_ptr(context));
    if (context == nullptr) {
        return nullptr;
    }

    return make_box<CommandListD3D11>(std::move(context));
}

void gm::gpu::d3d11::CommandListD3D11::setPipelineState(GpuPipelineState* state) {
    GM_ASSERT(state != nullptr);

    auto pipelineState = static_cast<PipelineStateD3D11*>(state);

    auto const& params = pipelineState->params();

    _context->IASetInputLayout(params.inputLayout.get());
    _context->RSSetState(params.rasterState.get());
    _context->VSSetShader(params.vertShader.get(), nullptr, 0);
    _context->PSSetShader(params.pixelShader.get(), nullptr, 0);
    _context->OMSetBlendState(params.blendState.get(), nullptr, ~UINT(0));
    _context->OMSetDepthStencilState(params.depthStencilState.get(), 0);
    _bindingsDirty = true;
}

void gm::gpu::d3d11::CommandListD3D11::bindRenderTarget(gm::uint32 index, GpuResourceView* view) {
    GM_ASSERT(index < maxRenderTargetBindings);

    if (view == nullptr) {
        _rtv[index].reset();
        return;
    }

    auto rtv = static_cast<ResourceViewD3D11*>(view);
    GM_ASSERT(rtv->type() == ViewType::RTV);

    _rtv[index] = rtv->getView().as<ID3D11RenderTargetView>();

    _bindingsDirty = true;
}

void gm::gpu::d3d11::CommandListD3D11::bindBuffer(gm::uint32 slot, Buffer* buffer, gm::uint64 stride, gm::uint64 offset) {
    GM_ASSERT(buffer != nullptr);
    GM_ASSERT(buffer->type() == BufferType::Vertex);

    auto d3dBuffer = static_cast<BufferD3D11*>(buffer);
    ID3D11Buffer* d3d11Buffer = static_cast<ID3D11Buffer*>(d3dBuffer->buffer().get());

    UINT d3dStride = static_cast<UINT>(stride);
    UINT d3dOffset = static_cast<UINT>(offset);
    _context->IASetVertexBuffers(slot, 1, &d3d11Buffer, &d3dStride, &d3dOffset);
}

void gm::gpu::d3d11::CommandListD3D11::bindShaderResource(gm::uint32 slot, GpuResourceView* view) {
    GM_ASSERT(view != nullptr);

    auto buffer = static_cast<ResourceViewD3D11*>(view);
    ID3D11ShaderResourceView* srv = static_cast<ID3D11ShaderResourceView*>(buffer->getView().get());

    _context->VSSetShaderResources(0, 1, &srv);
}

void gm::gpu::d3d11::CommandListD3D11::setPrimitiveTopology(PrimitiveTopology topology) {
    D3D11_PRIMITIVE_TOPOLOGY primitive;
    switch (topology) {
    case PrimitiveTopology::Triangles: primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
    }
    _context->IASetPrimitiveTopology(primitive);
}

void gm::gpu::d3d11::CommandListD3D11::setViewport(Viewport const& viewport) {
    D3D11_VIEWPORT d3d11Viewport;
    d3d11Viewport.TopLeftX = viewport.leftX;
    d3d11Viewport.TopLeftY = viewport.topY;
    d3d11Viewport.Width = viewport.width;
    d3d11Viewport.Height = viewport.height;
    d3d11Viewport.MinDepth = viewport.minDepth;
    d3d11Viewport.MaxDepth = viewport.maxDepth;
    _context->RSSetViewports(1, &d3d11Viewport);
}

void gm::gpu::d3d11::CommandListD3D11::draw(gm::uint32 vertexCount, gm::uint32 firstVertex) {
    _flushBindings();
    _context->Draw(vertexCount, firstVertex);
}

void gm::gpu::d3d11::CommandListD3D11::clearRenderTarget(GpuResourceView* view, PackedVector4f color) {
    GM_ASSERT(view != nullptr);

    _context->ClearRenderTargetView(static_cast<ID3D11RenderTargetView*>(static_cast<ResourceViewD3D11*>(view)->getView().get()), color);
}

void gm::gpu::d3d11::CommandListD3D11::finish() {
    _context->FinishCommandList(FALSE, out_ptr(_commands));
}

void gm::gpu::d3d11::CommandListD3D11::clear(GpuPipelineState* pipelineState) {
    _context->ClearState();
    _commands.reset();
    if (pipelineState != nullptr) {
        setPipelineState(pipelineState);
    }
}

auto gm::gpu::d3d11::CommandListD3D11::map(Buffer* resource, gm::uint64 size, gm::uint64 offset) -> span<gm::byte> {
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

void gm::gpu::d3d11::CommandListD3D11::unmap(Buffer* buffer, span<gm::byte const> data) {
    if (buffer == nullptr) {
        return;
    }

    ID3D11Buffer* d3dBuffer = static_cast<BufferD3D11*>(buffer)->buffer().get();

    _context->Unmap(d3dBuffer, 0);
}

void gm::gpu::d3d11::CommandListD3D11::update(Buffer* buffer, span<gm::byte const> data, gm::uint64 offset) {
    if (buffer == nullptr) {
        return;
    }

    auto target = map(buffer, data.size(), offset);
    std::memcpy(target.data(), data.data(), data.size());
    unmap(buffer, target);
}

void gm::gpu::d3d11::CommandListD3D11::_flushBindings() {
    if (_bindingsDirty) {
        _bindingsDirty = false;
        _context->OMSetRenderTargets(maxRenderTargetBindings, reinterpret_cast<ID3D11RenderTargetView**>(&_rtv), _dsv.get());
    }
}
