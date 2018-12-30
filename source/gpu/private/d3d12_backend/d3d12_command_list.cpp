// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_command_list.h"
#    include "d3d12_resource.h"
#    include "direct3d.h"
#    include "grimm/foundation/assert.h"
#    include "grimm/foundation/out_ptr.h"

gm::D3d12CommandList::D3d12CommandList(com_ptr<ID3D12CommandAllocator> allocator, com_ptr<ID3D12GraphicsCommandList> commands) : _allocator(std::move(allocator)), _commands(std::move(commands)) {}

gm::D3d12CommandList::~D3d12CommandList() {
    _commands.reset();
    _allocator.reset();
}

auto gm::D3d12CommandList::createCommandList(ID3D12Device1* device) -> box<D3d12CommandList> {
    com_ptr<ID3D12CommandAllocator> allocator;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), out_ptr(allocator));
    if (allocator == nullptr) {
        return nullptr;
    }

    com_ptr<ID3D12GraphicsCommandList> commands;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.get(), nullptr, __uuidof(ID3D12GraphicsCommandList), out_ptr(commands));
    if (commands == nullptr) {
        return nullptr;
    }

    commands->Close();

    return make_box<D3d12CommandList>(std::move(allocator), std::move(commands));
}

void gm::D3d12CommandList::clearRenderTarget(uint64 handle) {
    FLOAT rgba[] = {1, 0, 0, 1};
    _commands->ClearRenderTargetView({handle}, rgba, 0, nullptr);
}

void gm::D3d12CommandList::resourceBarrier(IGpuResource* resource, GpuResourceState from, GpuResourceState to) {
    GM_ASSERT(resource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE};
    barrier.Transition.pResource = static_cast<D3d12Resource*>(resource)->get().get();
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = toD3d12State(from);
    barrier.Transition.StateAfter = toD3d12State(to);
    _commands->ResourceBarrier(1, &barrier);
}

void gm::D3d12CommandList::reset() {
    _allocator->Reset();
    _commands->Reset(_allocator.get(), nullptr);
}

D3D12_RESOURCE_STATES gm::D3d12CommandList::toD3d12State(GpuResourceState state) {
    switch (state) {
    case GpuResourceState::Present:
        return D3D12_RESOURCE_STATE_PRESENT;
    case GpuResourceState::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    default:
        GM_ASSERT(false, "Invalid gpu resource state {0}", (int)state);
        return D3D12_RESOURCE_STATE_COMMON;
    }
}

#endif
