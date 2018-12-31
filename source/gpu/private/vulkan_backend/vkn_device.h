// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
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

    class VknDevice final : public IGPUDevice {
    public:
        struct Context {
            vk::UniqueInstance instance;
            vk::PhysicalDevice physicalDevice;
            vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic> device;
            box<vk::DispatchLoaderDynamic> loader;
        };

        VknDevice(Context context);
        virtual ~VknDevice();

        static box<IGPUDevice> createDevice();

        box<ISwapChain> createSwapChain(void* native_window) override;
        box<IDescriptorHeap> createDescriptorHeap() override;
        box<ICommandList> createCommandList(IPipelineState* pipelineState = nullptr) override;
        box<IPipelineState> createPipelineState() override;

        void execute(ICommandList* commandList) override;

        void createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) override;

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
