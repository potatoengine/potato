// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_VULKAN

#    include "vkn_swap_chain.h"
#    include "grimm/gpu/resource.h"

gm::VknSwapChain::VknSwapChain(vk::UniqueHandle<vk::SurfaceKHR, vk::DispatchLoaderDynamic> surface, vk::UniqueHandle<vk::SwapchainKHR, vk::DispatchLoaderDynamic> swapchain)
    : _surface(std::move(surface)), _swapchain(std::move(swapchain)) {}

gm::VknSwapChain::~VknSwapChain() = default;

auto gm::VknSwapChain::createSwapChain(vk::Instance instance, vk::Device device, vk::DispatchLoaderDynamic& loader, void* nativeWindow) -> box<VknSwapChain> {
    GM_ASSERT(nativeWindow != nullptr);

#    if _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hwnd = static_cast<HWND>(nativeWindow);
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

    VkSurfaceKHR surfaceTmp;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surfaceTmp);
    vk::UniqueHandle<vk::SurfaceKHR, vk::DispatchLoaderDynamic> surface{surfaceTmp};
#    endif

    vk::SwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.surface = surface.get();

    auto [result2, swapchain] = device.createSwapchainKHRUnique(swapchainCreateInfo, nullptr, loader);

    return make_box<VknSwapChain>(std::move(surface), std::move(swapchain));
}

void gm::VknSwapChain::present() {
}

void gm::VknSwapChain::resizeBuffers(int width, int height) {}

auto gm::VknSwapChain::getBuffer(int index) -> box<GpuResource> {
    return nullptr;
}

int gm::VknSwapChain::getCurrentBufferIndex() {
    return 0;
}

#endif
