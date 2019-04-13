// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/box.h"
#include "potato/gpu/command_list.h"

namespace up::gpu::d3d11 {
    class CommandListD3D11 final : public GpuCommandList {
    public:
        CommandListD3D11(com_ptr<ID3D11DeviceContext> context);
        virtual ~CommandListD3D11();

        CommandListD3D11(CommandListD3D11&&) = delete;
        CommandListD3D11& operator=(CommandListD3D11&&) = delete;

        static box<CommandListD3D11> createCommandList(ID3D11Device* device, PipelineState* pipelineState);

        void setPipelineState(PipelineState* state) override;

        void bindRenderTarget(uint32 index, ResourceView* view) override;
        void bindDepthStencil(ResourceView* view) override;
        void bindIndexBuffer(GpuBuffer* buffer, IndexType indexType, uint32 offset = 0) override;
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override;
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, ShaderStage stage) override;
        void bindShaderResource(uint32 slot, ResourceView* view, ShaderStage stage) override;
        void bindSampler(uint32 slot, Sampler* sampler, ShaderStage stage) override;
        void setClipRect(Rect rect) override;

        void setPrimitiveTopology(PrimitiveTopology topology) override;
        void setViewport(Viewport const& viewport) override;

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override;
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override;

        void clearRenderTarget(ResourceView* view, glm::vec4 color) override;
        void clearDepthStencil(ResourceView* view) override;

        void finish() override;
        void clear(PipelineState* pipelineState = nullptr) override;

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override;
        void unmap(GpuBuffer* resource, span<byte const> data) override;
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override;

        com_ptr<ID3D11DeviceContext> const& deviceContext() const { return _context; }
        com_ptr<ID3D11CommandList> const& commandList() const { return _commands; }

    private:
        void _flushBindings();

        static constexpr uint32 maxRenderTargetBindings = 4;

        com_ptr<ID3D11DeviceContext> _context;
        com_ptr<ID3D11RenderTargetView> _rtv[maxRenderTargetBindings];
        com_ptr<ID3D11DepthStencilView> _dsv;
        com_ptr<ID3D11CommandList> _commands;
        bool _bindingsDirty = false;
    };
} // namespace up::gpu::d3d11
