// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_command_list.h"
#include "d3d11_pipeline_state.h"
#include "d3d11_resource.h"
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

void gm::CommandListD3D11::clearRenderTarget(gm::uint64 handle, PackedVector4f color) {
    _context->ClearRenderTargetView(nullptr, color);
}

void gm::CommandListD3D11::resourceBarrier(GpuResource* resource, GpuResourceState from, GpuResourceState to) {
}

void gm::CommandListD3D11::reset(GpuPipelineState* pipelineState) {
    _context->ClearState();
}
