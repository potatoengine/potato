// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_VULKAN

#    include "vkn_device.h"
#    include "grimm/foundation/assert.h"
#    include "grimm/foundation/logging.h"
#    include "grimm/foundation/out_ptr.h"
#    include "vkn_pipeline_state.h"
#    include "vkn_swap_chain.h"
#    include <array>

gm::VknDevice::VknDevice(Context context) : _context(std::move(context)) {}

gm::VknDevice::~VknDevice() = default;

auto gm::VknDevice::createDevice() -> box<IGPUDevice> {
    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = "Grimm";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "Grimm";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    auto instanceLayers = std::array{
        "VK_LAYER_LUNARG_standard_validation",
    };
    auto instanceExtensions = std::array{
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = instanceLayers.size();
    createInfo.ppEnabledLayerNames = instanceLayers.data();
    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    auto [result, instance] = vk::createInstanceUnique(createInfo, nullptr);
    if (result != vk::Result::eSuccess) {
        return nullptr;
    }

    auto loader = vk::DispatchLoaderDynamic(instance.get());

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    debugCreateInfo.pfnUserCallback = _debugCallback;
    debugCreateInfo.pUserData = nullptr;

    auto [result3, debug] = instance->createDebugUtilsMessengerEXTUnique(debugCreateInfo, nullptr, loader);

    vk::PhysicalDevice physicalDevice;
    uint32 physicalDeviceCount = 1;
    result = instance->enumeratePhysicalDevices(&physicalDeviceCount, &physicalDevice, loader);

    auto deviceExtensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    auto [result2, device] = physicalDevice.createDeviceUnique(deviceCreateInfo, nullptr, loader);

    loader = vk::DispatchLoaderDynamic(instance.get(), device.get());

    VknDevice::Context context;
    context.instance = std::move(instance);
    context.device = std::move(device);
    context.physicalDevice = std::move(physicalDevice);
    context.loader = make_box<vk::DispatchLoaderDynamic>(std::move(loader));

    return make_box<VknDevice>(std::move(context));
}

auto gm::VknDevice::createSwapChain(void* nativeWindow) -> box<ISwapChain> {
    GM_ASSERT(nativeWindow != nullptr);

    return VknSwapChain::createSwapChain(_context.instance.get(), _context.device.get(), *_context.loader, nativeWindow);
}

auto gm::VknDevice::createDescriptorHeap() -> box<IDescriptorHeap> {
    GM_ASSERT(false, "Unsupported");
    return nullptr;
}

auto gm::VknDevice::createCommandList(IPipelineState* pipelineState) -> box<ICommandList> {
    GM_ASSERT(false, "Unsupported");
    return nullptr;
}

void gm::VknDevice::createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) {
    GM_ASSERT(renderTarget != nullptr);
    GM_ASSERT(false, "Unsupported");
}

auto gm::VknDevice::createPipelineState() -> box<IPipelineState> {
    GM_ASSERT(false, "Unsupported");
    return nullptr;
}

void gm::VknDevice::execute(ICommandList* commandList) {
    GM_ASSERT(commandList != nullptr);
    GM_ASSERT(false, "Unsupported");
}

VKAPI_ATTR VkBool32 VKAPI_CALL gm::VknDevice::_debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        GM_LOG_ERROR("Vulkan [{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }
    else {
        GM_LOG_DEBUG("Vulkan [{}]: {}", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    }
    return VK_FALSE;
}

#endif
