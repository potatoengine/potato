// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/gpu/swap_chain.h"
#include "vulkan.h"

namespace gm {
    class VknSwapChain final : public GpuSwapChain {
    public:
        VknSwapChain(vk::UniqueHandle<vk::SurfaceKHR, vk::DispatchLoaderDynamic> surface, vk::UniqueHandle<vk::SwapchainKHR, vk::DispatchLoaderDynamic> swapchain);
        virtual ~VknSwapChain();

        static box<VknSwapChain> createSwapChain(vk::Instance instance, vk::Device device, vk::DispatchLoaderDynamic& loader, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        box<GpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        vk::UniqueHandle<vk::SurfaceKHR, vk::DispatchLoaderDynamic> _surface;
        vk::UniqueHandle<vk::SwapchainKHR, vk::DispatchLoaderDynamic> _swapchain;
    };
} // namespace gm
