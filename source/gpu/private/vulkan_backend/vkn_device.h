// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/gpu/command_list.h"
#include "grimm/gpu/descriptor_heap.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/resource.h"
#include "grimm/gpu/swap_chain.h"
#include "vulkan.h"

namespace gm {
    class VknDevice;
    class VkSwapChain;
    class VkResource;
    class VkCommandList;
    class VkDescriptorHeap;

    class VknDevice final : public GpuDevice {
    public:
        struct Context {
            vk::UniqueInstance instance;
            vk::PhysicalDevice physicalDevice;
            vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic> device;
            box<vk::DispatchLoaderDynamic> loader;
        };

        VknDevice(Context context);
        virtual ~VknDevice();

        static box<GpuDevice> createDevice();

        box<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuDescriptorHeap> createDescriptorHeap() override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState() override;

        void execute(GpuCommandList* commandList) override;

        void createRenderTargetView(GpuResource* renderTarget, uint64 cpuHandle) override;

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        Context _context;
    };
} // namespace gm
