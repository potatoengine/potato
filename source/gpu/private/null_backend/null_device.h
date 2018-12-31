// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "device.h"

namespace gm {
    class NullDevice final : public IGPUDevice {
    public:
        NullDevice();
        virtual ~NullDevice();

        box<ISwapChain> createSwapChain(void* native_window) override;
        box<IDescriptorHeap> createDescriptorHeap() override;
        box<ICommandList> createCommandList(IPipelineState* pipelineState = nullptr) override;
        box<IPipelineState> createPipelineState() override;

        void createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) override;

        void execute(ICommandList* commands) override;
    };
} // namespace gm
