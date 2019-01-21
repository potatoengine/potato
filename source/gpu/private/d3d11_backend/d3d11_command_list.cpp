// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource.h"
#include "d3d11_resource_view.h"
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

void gm::CommandListD3D11::clearRenderTarget(GpuResourceView* view, PackedVector4f color) {
    GM_ASSERT(view != nullptr);

    _context->ClearRenderTargetView(static_cast<ID3D11RenderTargetView*>(static_cast<ResourceViewD3D11*>(view)->getView().get()), color);
}

void gm::CommandListD3D11::resourceBarrier(GpuResource* resource, GpuResourceState from, GpuResourceState to) {
}

void gm::CommandListD3D11::reset(GpuPipelineState* pipelineState) {
    _context->ClearState();
}
