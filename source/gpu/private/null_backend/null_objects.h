// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "command_list.h"
#include "descriptor_heap.h"
#include "device.h"
#include "factory.h"
#include "pipeline_state.h"
#include "resource.h"
#include "swap_chain.h"

namespace gm {
    class NullDevice;

    class NullFactory final : public IGPUFactory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<IGPUDevice> createDevice(int index) override;
    };

    class NullDevice final : public IGPUDevice {
    public:
        box<ISwapChain> createSwapChain(void* native_window) override;
        box<IDescriptorHeap> createDescriptorHeap() override;
        box<ICommandList> createCommandList(IPipelineState* pipelineState = nullptr) override;
        box<IPipelineState> createPipelineState() override;

        void createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) override {}

        void execute(ICommandList* commands) override {}
    };

    class NullSwapChain final : public ISwapChain {
    public:
        void present() override {}
        void resizeBuffers(int width, int height) override {}
        box<IGpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class NullPipelineState final : public IPipelineState {
    };

    class NullResource final : public IGpuResource {
    };

    class NullDescriptorHeap final : public IDescriptorHeap {
    public:
        DescriptorHandle getCpuHandle() const override;
    };

    class NullCommandList final : public ICommandList {
    public:
        void clearRenderTarget(uint64 handle) override {}
        void resourceBarrier(IGpuResource* resource, GpuResourceState from, GpuResourceState to) override {}

        void reset(IPipelineState* pipelineState = nullptr) override {}
    };
} // namespace gm
