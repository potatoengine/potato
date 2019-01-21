// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/command_list.h"

namespace gm {
    class CommandListD3D11 final : public GpuCommandList {
    public:
        CommandListD3D11(com_ptr<ID3D11DeviceContext> context);
        virtual ~CommandListD3D11();

        CommandListD3D11(CommandListD3D11&&) = delete;
        CommandListD3D11& operator=(CommandListD3D11&&) = delete;

        static box<CommandListD3D11> createCommandList(ID3D11Device* device, GpuPipelineState* pipelineState);

        void bindRenderTarget(uint32 index, GpuResourceView* view) override;
        void bindBuffer(uint32 slot, GpuResourceView* view) override;

        void clearRenderTarget(GpuResourceView* view, PackedVector4f color) override;

        void clear(GpuPipelineState* pipelineState = nullptr) override;

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override;
        void unmap(GpuBuffer* resource, span<byte const> data) override;
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override;

        com_ptr<ID3D11DeviceContext> const& deviceContext() const { return _context; }

    private:
        static constexpr uint32 maxRenderTargetBindings = 4;

        com_ptr<ID3D11DeviceContext> _context;
        com_ptr<ID3D11RenderTargetView> _rtv[maxRenderTargetBindings];
        com_ptr<ID3D11DepthStencilView> _dsv;
    };
} // namespace gm
