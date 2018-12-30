// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/command_list.h"

namespace gm {
    class D3d12CommandList final : public ICommandList {
    public:
        D3d12CommandList(com_ptr<ID3D12CommandAllocator> allocator, com_ptr<ID3D12GraphicsCommandList> commands);
        virtual ~D3d12CommandList();

        D3d12CommandList(D3d12CommandList&&) = delete;
        D3d12CommandList& operator=(D3d12CommandList&&) = delete;

        static box<D3d12CommandList> createCommandList(ID3D12Device1* device);

        void clearRenderTarget(uint64 handle) override;
        void resourceBarrier(IGpuResource* resource, GpuResourceState from, GpuResourceState to) override;
        void reset() override;

        com_ptr<ID3D12GraphicsCommandList> const& get() const { return _commands; }

    private:
        static D3D12_RESOURCE_STATES toD3d12State(GpuResourceState state);

    private:
        com_ptr<ID3D12CommandAllocator> _allocator;
        com_ptr<ID3D12GraphicsCommandList> _commands;
    };
} // namespace gm
