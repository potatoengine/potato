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

        void clearRenderTarget(uint64 handle, PackedVector4f color) override;
        void resourceBarrier(GpuResource* resource, GpuResourceState from, GpuResourceState to) override;
        void reset(GpuPipelineState* pipelineState = nullptr) override;

        com_ptr<ID3D11DeviceContext> const& getDeviceContext() const { return _context; }

    private:
        com_ptr<ID3D11DeviceContext> _context;
    };
} // namespace gm
