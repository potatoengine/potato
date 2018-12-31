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

    class NullFactory final : public GpuDeviceFactory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<GpuDevice> createDevice(int index) override;
    };

    class NullDevice final : public GpuDevice {
    public:
        box<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuDescriptorHeap> createDescriptorHeap() override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState() override;

        void createRenderTargetView(GpuResource* renderTarget, uint64 cpuHandle) override {}

        void execute(GpuCommandList* commands) override {}
    };

    class NullSwapChain final : public GpuSwapChain {
    public:
        void present() override {}
        void resizeBuffers(int width, int height) override {}
        box<GpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class NullPipelineState final : public GpuPipelineState {
    };

    class NullResource final : public GpuResource {
    };

    class NullDescriptorHeap final : public GpuDescriptorHeap {
    public:
        GpuDescriptorHandle getCpuHandle() const override;
    };

    class NullCommandList final : public GpuCommandList {
    public:
        void clearRenderTarget(uint64 handle) override {}
        void resourceBarrier(GpuResource* resource, GpuResourceState from, GpuResourceState to) override {}

        void reset(GpuPipelineState* pipelineState = nullptr) override {}
    };
} // namespace gm
