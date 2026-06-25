#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanSystem.h"
#include <cstdlib>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "MemorySystem.h"
#include <Platform.h>
#include "EngineConfigSystem.h"
#include "BufferSystem.h"
#if defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

VulkanSystem& vulkanSystem = VulkanSystem::Get();
//LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;
//
//void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
//{
//    g_logVulkanMessageCallback = callback;
//}
//
//void VulkanSystem_LogVulkanMessage(const char* message, int severity)
//{
//    if (g_logVulkanMessageCallback)
//    {
//        g_logVulkanMessageCallback(message, severity);
//    }
//}

uint32 VulkanSystem::FindMaxApiVersion(VkPhysicalDevice physicalDevice)
{
    uint32 version = GetPhysicalDeviceProperties(physicalDevice).apiVersion;
#ifndef __ANDROID__
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 4)) return VK_API_VERSION_1_4;
#endif
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 3)) return VK_API_VERSION_1_3;
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 2)) return VK_API_VERSION_1_2;
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 1)) return VK_API_VERSION_1_1;
}

void VulkanSystem::VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution)
{
   // vulkanWindow.Create("Game", windowResolution.x, windowResolution.y);

    m_usingCustomSurface = false;
    DefaultRenderPassResolution = windowResolution;
    vulkan.VulkanSetUp(windowResolution, renderResolution);
    RendererSetUp(vulkan.WindowHandle(), DefaultRenderPassResolution);
}

void VulkanSystem::VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution)
{
    m_usingCustomSurface = true;
    DefaultRenderPassResolution = windowResolution;
    vulkan.VulkanSetUp(windowResolution, renderResolution);
    RendererSetUp(vulkan.WindowHandle(), DefaultRenderPassResolution);
}

void VulkanSystem::RendererSetUp(const void* windowHandle, ivec2 renderResolution)
{
    DefaultRenderPassResolution = configSystem.RenderResolution;

    vulkanSystem.ImageIndex = 0;
    vulkanSystem.CommandIndex = 0;
    vulkanSystem.RebuildRendererFlag = false;


    bufferSystem.vmaAllocator = SetUpVmaAllocation();
    SetUpSwapChain(windowHandle);
    vulkanSystem.CommandPool = SetUpCommandPool(vulkan.LogicalDevice(), vulkan.Device().GraphicsFamily());
    SetUpCommandBuffers();
    SetUpSemaphores();

#if defined(__ANDROID__)
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(Device, "vkGetBufferDeviceAddress");
    if (vkGetBufferDeviceAddress == nullptr) {
        throw std::runtime_error("Failed to load vkGetBufferDeviceAddress function pointer!");
    }
#endif

}

VkExtent2D VulkanSystem::SetUpSwapChainExtent(const void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
#ifndef PLATFORM_ANDROID
    surfaceCapabilities = GetSurfaceCapabilities(vulkan.PhysicalDevice(), vulkan.Surface());
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return surfaceCapabilities.currentExtent;
    }

    VkExtent2D extent = { static_cast<uint32_t>(surfaceCapabilities.currentExtent.width), static_cast<uint32_t>(surfaceCapabilities.currentExtent.height) };
#else
    VkExtent2D extent = {
            surfaceCapabilities.currentExtent.width,
            surfaceCapabilities.currentExtent.height
    };
    if (extent.width == UINT32_MAX) {
        extent = { 1280, 720 };
    }
#endif
    extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
    return extent;
}

void VulkanSystem::SetUpSwapChain(const void* windowHandle)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = GetPhysicalDeviceFormats(vulkan.PhysicalDevice(), vulkan.Surface());
    VkExtent2D extent = SetUpSwapChainExtent(windowHandle, surfaceCapabilities);
    Vector<VkPresentModeKHR> compatiblePresentModesList = GetPhysicalDevicePresentModes(vulkan.PhysicalDevice(), vulkan.Surface());
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);
    
    SetUpSwapChain();
    SetUpSwapChainImages();
    SetUpSwapChainImageViews(swapChainImageFormat);
}

VmaAllocator VulkanSystem::SetUpVmaAllocation()
{
    VmaVulkanFunctions vulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };

    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = vulkan.PhysicalDevice(),
        .device = vulkan.LogicalDevice(),
        .preferredLargeHeapBlockSize = 64ull << 20,   // 64 MB
        .pVulkanFunctions = &vulkanFunctions,
        .instance = vulkan.InstanceHandle(),
        .vulkanApiVersion = vulkan.ApiVersion(),
    };

    VmaAllocator allocator = VK_NULL_HANDLE;
    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
    }

    return allocator;
}

void VulkanSystem::DestroyRenderer()
{
    DestroySwapChainImageView(vulkan.LogicalDevice(), vulkanSystem.SwapChainImageViews);
    DestroySwapChain(vulkan.LogicalDevice(), &vulkanSystem.Swapchain);
    DestroyFences(vulkan.LogicalDevice(), vulkanSystem.AcquireImageSemaphores, vulkanSystem.PresentImageSemaphores, vulkanSystem.InFlightFences);
    DestroyCommandPool(vulkan.LogicalDevice(), &vulkanSystem.CommandPool);
    vmaDestroyAllocator(bufferSystem.vmaAllocator); 
    DestroyDevice(vulkan.LogicalDevice());
    //DestroyDebugger(m_instance.InstanceHandle(), vulkanSystem.DebugMessenger);
    //DestroySurface(m_instance.InstanceHandle(), &m_instance.Surface());
    //DestroyInstance(m_instance.InstanceHandle());
}

Vector<const char*> VulkanSystem::GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32 count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, availableDeviceExtensions.data());

    std::vector<const char*> enabledDeviceExtensions;
    auto AddDeviceExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& deviceExtensions : availableDeviceExtensions)
            {
                if (strcmp(deviceExtensions.extensionName, ext) == 0)
                {
                    enabledDeviceExtensions.push_back(ext);
                    std::cout << "[Device] Enabling: " << ext << '\n';
                    return true;
                }
            }
            std::cout << "[Device] Missing:   " << ext << '\n';
            return false;
        };

    if (!AddDeviceExtensionIfSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
    {
        throw std::runtime_error("FATAL: Swapchain extension not supported!");
    }
    AddDeviceExtensionIfSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    return enabledDeviceExtensions;
}

VkBool32 VKAPI_CALL VulkanSystem::DebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT      MessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             MessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* CallBackData,
    void* UserData)
{
    const char* severityStr = "";
    const char* colorCode = "";

    switch (MessageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        severityStr = "VERBOSE";
        colorCode = "\033[34m";  // blue
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        severityStr = "INFO";
        colorCode = "\033[32m";  // green
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        severityStr = "WARNING";
        colorCode = "\033[33m";  // yellow
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        severityStr = "ERROR";
        colorCode = "\033[31m";  // red
        break;
    default:
        severityStr = "UNKNOWN";
        colorCode = "\033[35m";  // magenta
        break;
    }

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD originalAttributes = 7;

    if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &consoleInfo)) originalAttributes = consoleInfo.wAttributes;

    WORD color = originalAttributes;
    if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   color = FOREGROUND_RED;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) color = FOREGROUND_RED | FOREGROUND_GREEN;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    color = FOREGROUND_GREEN;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) color = FOREGROUND_BLUE;

    SetConsoleTextAttribute(hConsole, color);
    fprintf(stderr, "%s: ", severityStr);
    SetConsoleTextAttribute(hConsole, originalAttributes);
    fprintf(stderr, "%s\n", CallBackData->pMessage);
#else
    // Linux / macOS: ANSI escape codes (works in every terminal)
    fprintf(stderr, "%s%s: \033[0m%s\n", colorCode, severityStr, CallBackData->pMessage);
#endif
    //VulkanSystem_LogVulkanMessage(CallBackData->pMessage, static_cast<int>(MessageSeverity));
    return VK_FALSE;
}

Vector<VkSurfaceFormatKHR> VulkanSystem::GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR>  surfaceFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormatList.data()));
    return surfaceFormatList;
}

Vector<VkPresentModeKHR> VulkanSystem::GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL));
    Vector<VkPresentModeKHR> presentModeList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModeList.data()));
    return presentModeList;
}

Vector<const char*> VulkanSystem::GetRequiredInstanceExtensions()
{
    uint32 count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    Vector<VkExtensionProperties> availableExtensionList(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensionList.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& extension : availableExtensionList)
                if (strcmp(extension.extensionName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            std::cout << "Extension not supported: " << ext << '\n';
        };

    AddExtensionIfSupported(VK_KHR_SURFACE_EXTENSION_NAME);
    AddExtensionIfSupported(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
#if defined(_WIN32)
    AddExtensionIfSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) && !defined(__ANDROID__)
    AddExtensionIfSupported("VK_KHR_xcb_surface");
    AddExtensionIfSupported("VK_KHR_wayland_surface");
#elif defined(__ANDROID__)
    AddExtensionIfSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if !defined(NDEBUG) && !defined(__ANDROID__)
    AddExtensionIfSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    if (extensions.empty() ||
        (extensions.size() == 1 && extensions[0] == VK_KHR_SURFACE_EXTENSION_NAME))
    {
        throw std::runtime_error("No platform surface extension available — cannot create window.");
    }
    return extensions;
}

VkPhysicalDeviceFeatures VulkanSystem::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    return features;
}

VkPhysicalDeviceFeatures2 VulkanSystem::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures2 features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    return features;
}

Vector<VkPhysicalDevice> VulkanSystem::GetPhysicalDeviceList(VkInstance& instance)
{
    uint32 deviceCount = 0;
    Vector<VkPhysicalDevice> physicalDeviceList = Vector<VkPhysicalDevice>();
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    physicalDeviceList = Vector<VkPhysicalDevice>(deviceCount);
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data()));
    return physicalDeviceList;
}

VkCommandPool VulkanSystem::SetUpCommandPool(VkDevice device, uint32 graphicsFamily)
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo CommandPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsFamily
    };
    VULKAN_THROW_IF_FAIL(vkCreateCommandPool(device, &CommandPoolCreateInfo, NULL, &commandPool));
    return commandPool;
}

void VulkanSystem::GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue& graphicsQueue, VkQueue& presentQueue)
{
    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX) {
        fprintf(stderr, "ERROR: Invalid queue family index!\n");
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);

    if (graphicsQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: graphicsQueue is NULL! Family: %u (index 0 invalid?)\n", graphicsFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }
    if (presentQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: presentQueue is NULL! Family: %u (index 0 invalid?)\n", presentFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    printf("SUCCESS: GraphicsQueue = %p (family %u), PresentQueue = %p (family %u)\n",
        (void*)graphicsQueue, graphicsFamily, (void*)presentQueue, presentFamily);
}

VkSurfaceFormatKHR VulkanSystem::FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (uint32 x = 0; x < availableFormats.size(); x++)
    {
        if (availableFormats[x].format == VK_FORMAT_R8G8B8A8_UNORM &&
            availableFormats[x].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormats[x];
        }
    }
    fprintf(stderr, "Couldn't find a usable swap surface format.\n");
    return VkSurfaceFormatKHR{ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
}

VkPresentModeKHR VulkanSystem::FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
{
    for (uint32 x = 0; x < availablePresentModes.size(); x++)
    {
        if (availablePresentModes[x] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentModes[x];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanSystem::GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    Vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, families.data());

    for (uint32 x = 0; x < queueFamilyCount; x++)
    {
        if (families[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = x;

            VkBool32 presentSupport = VK_FALSE;
            VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, surface, &presentSupport));
            if (presentSupport) presentFamily = x;
            if (graphicsFamily != UINT32_MAX) break;
        }
    }
}

void VulkanSystem::GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList)
{
    if (RequestRayTracingSupport)
    {
        VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures{};
        AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures{};
        RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        RayTracingPipelineFeatures.pNext = &AccelerationStructureFeatures;

        VkPhysicalDeviceFeatures2 DeviceFeatures2{};
        DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        DeviceFeatures2.pNext = &RayTracingPipelineFeatures;
        vkGetPhysicalDeviceFeatures2(gpuDevice, &DeviceFeatures2);

        if (RayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
            AccelerationStructureFeatures.accelerationStructure == VK_TRUE)
        {
            if (std::find(featureList.begin(), featureList.end(), VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) != featureList.end() &&
                std::find(featureList.begin(), featureList.end(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) != featureList.end())
            {
                RayTracingSupported = true;
                deviceExtensionList.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                deviceExtensionList.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            }
            else
            {
                RayTracingSupported = false;
            }
        }
        else
        {
            std::cout << "GPU/MotherBoard isn't ray tracing compatible." << std::endl;
        }
    }
}

VkSampleCountFlagBits VulkanSystem::GetMaxSampleCount(VkPhysicalDevice gpuDevice)
{
    VkPhysicalDeviceLimits physicalDeviceProperties = GetPhysicalDeviceProperties(vulkan.PhysicalDevice()).limits;
    VkSampleCountFlags counts = physicalDeviceProperties.framebufferColorSampleCounts & physicalDeviceProperties.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

VkSurfaceCapabilitiesKHR VulkanSystem::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
    return surfaceCapabilities;
}

VkPhysicalDeviceProperties VulkanSystem::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    return props;
}

Vector<VkSurfaceFormatKHR> VulkanSystem::GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, compatibleSwapChainFormatList.data()));
    return compatibleSwapChainFormatList;
}

Vector<const char*> VulkanSystem::GetValidationLayerProperties()
{
    uint32 layerCount = UINT32_MAX;
    Vector<const char*> validationLayers;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& layer : availableLayers)
            {
                if (strcmp(layer.layerName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            }
            std::cout << "Extension not supported: " << ext << '\n';
            //   __android_log_print(ANDROID_LOG_WARN, "Vulkan", "Validation layers not available - running without validation");
        };
    AddExtensionIfSupported("VK_LAYER_KHRONOS_validation");

    return extensions;
}

Vector<VkPresentModeKHR> VulkanSystem::GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    Vector<VkPresentModeKHR> compatiblePresentModesList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, compatiblePresentModesList.data()));
    return compatiblePresentModesList;
}

void VulkanSystem::SetUpSwapChain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities(vulkan.PhysicalDevice(), vulkan.Surface());
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = GetPhysicalDeviceFormats(vulkan.PhysicalDevice(), vulkan.Surface());
    Vector<VkPresentModeKHR> compatiblePresentModesList = GetPhysicalDevicePresentModes(vulkan.PhysicalDevice(), vulkan.Surface());
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);

    uint32 imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0)
    {
        imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);
    }
    imageCount = std::max(imageCount, surfaceCapabilities.minImageCount);

    VkExtent2D extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width = std::clamp(static_cast<uint32>(configSystem.WindowResolution.x), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32>(configSystem.WindowResolution.y), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    SwapChainResolution = extent;
    SwapChainImageCount = imageCount;
    MaxFramesInFlight = SwapChainImageCount;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkan.Surface(),
        .minImageCount = static_cast<uint32>(vulkanSystem.SwapChainImageCount),
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = vulkanSystem.Swapchain
    };

    if (vulkan.Device().GraphicsFamily() != vulkan.Device().PresentFamily())
    {
        Vector<uint32> queueFamilyIndices =
        {
            vulkan.Device().GraphicsFamily(),
            vulkan.Device().PresentFamily()
        };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_THROW_IF_FAIL(vkCreateSwapchainKHR(vulkan.LogicalDevice(), &SwapChainCreateInfo, nullptr, &vulkanSystem.Swapchain));
}

void VulkanSystem::RebuildSwapChain(const void* windowHandle)
{
}

void VulkanSystem::SetUpSwapChainImages()
{
    uint32 swapChainImageCount = UINT32_MAX;
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.LogicalDevice(), Swapchain, &swapChainImageCount, nullptr));
    SwapChainImages.resize(swapChainImageCount);
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.LogicalDevice(), Swapchain, &swapChainImageCount, SwapChainImages.data()));
}

void VulkanSystem::SetUpSwapChainImageViews(VkSurfaceFormatKHR swapChainImageFormat)
{
    SwapChainImageViews.resize(SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < SwapChainImageCount; x++)
    {
        VkImageViewCreateInfo swapChainViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = SwapChainImages[x],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat.format,
            .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkan.LogicalDevice(), &swapChainViewInfo, nullptr, &SwapChainImageViews[x]));
    }
}

void VulkanSystem::SetUpSemaphores()
{
    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = NULL,
        .semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCreateInfo
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };


    InFlightFences.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    AcquireImageSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    PresentImageSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    for (int x = 0; x < MaxFramesInFlight; x++)
    {
        VULKAN_THROW_IF_FAIL(vkCreateFence(vulkan.LogicalDevice(), &fenceInfo, NULL, &InFlightFences[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.LogicalDevice(), &semaphoreCreateInfo, NULL, &AcquireImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.LogicalDevice(), &semaphoreCreateInfo, NULL, &PresentImageSemaphores[x]));
    }
}

void VulkanSystem::SetUpCommandBuffers()
{
    CommandBuffers = Vector<VkCommandBuffer>(SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < SwapChainImageCount; x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = vulkanSystem.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32>(SwapChainImageCount)
        };

        vkAllocateCommandBuffers(vulkan.LogicalDevice(), &commandBufferAllocateInfo, &CommandBuffers[x]);
    }
}

uint32_t VulkanSystem::GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t x = 0; x < memProperties.memoryTypeCount; x++)
    {
        if ((typeFilter & (1 << x)) &&
            (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
        {
            return x;
        }
    }

    return UINT32_MAX;
}

VkCommandBuffer VulkanSystem::BeginSingleUseCommand()
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkan.LogicalDevice(), &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

void VulkanSystem::EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkan.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_THROW_IF_FAIL(vkQueueWaitIdle(vulkan.GraphicsQueue()));

    vkFreeCommandBuffers(vulkan.LogicalDevice(), CommandPool, 1, &commandBuffer);
}

void VulkanSystem::StartFrame()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vulkanSystem.CommandIndex = (vulkanSystem.CommandIndex + 1) % vulkanSystem.SwapChainImageCount;

    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkan.LogicalDevice(), 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex], VK_TRUE, UINT64_MAX));
    VULKAN_THROW_IF_FAIL(vkResetFences(vulkan.LogicalDevice(), 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]));
    VULKAN_THROW_IF_FAIL(vkResetCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], 0));
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], &commandBufferBeginInfo));
    VkResult result = vkAcquireNextImageKHR(vulkan.LogicalDevice(), vulkanSystem.Swapchain, UINT64_MAX, vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex], VK_NULL_HANDLE, &vulkanSystem.ImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
}

void VulkanSystem::EndFrame(VkCommandBuffer& commandBufferSubmit)
{
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBufferSubmit,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.CommandIndex]
    };

    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex]));
    VkResult submitResult = vkQueueSubmit(vulkan.GraphicsQueue(), 1, &submitInfo, vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]);
    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
        submitResult == VK_SUBOPTIMAL_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
        return;
    }
    else if (submitResult != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(submitResult);
    }

    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.CommandIndex],
        .swapchainCount = 1,
        .pSwapchains = &vulkanSystem.Swapchain,
        .pImageIndices = &vulkanSystem.ImageIndex
    };

    VkResult result = vkQueuePresentKHR(vulkan.PresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
        return;
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
}

void VulkanSystem::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, debugUtilsMessengerEXT, pAllocator);
    }
    else
    {
        fprintf(stderr, "Failed to load vkDestroyDebugUtilsMessengerEXT function\n");
    }
}

void VulkanSystem::DestroyFences(VkDevice device, Vector<VkSemaphore>& acquireImageSemaphores, Vector<VkSemaphore>& presentImageSemaphores, Vector<VkFence>& fences)
{
    for (auto& acquireImageSemaphore : acquireImageSemaphores)
    {
        if (acquireImageSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, acquireImageSemaphore, NULL);
            acquireImageSemaphore = VK_NULL_HANDLE;
        }
    }
    acquireImageSemaphores.clear();

    for (auto& presentImageSemaphore : presentImageSemaphores)
    {
        if (presentImageSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, presentImageSemaphore, NULL);
            presentImageSemaphore = VK_NULL_HANDLE;
        }
    }
    presentImageSemaphores.clear();

    for (auto& fence : fences)
    {
        if (fence != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fence, NULL);
            fence = VK_NULL_HANDLE;
        }
    }
    fences.clear();
}

void VulkanSystem::DestroyCommandPool(VkDevice device, VkCommandPool* commandPool)
{
    if (*commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, *commandPool, NULL);
        *commandPool = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDevice(VkDevice device)
{
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, NULL);
        device = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (*surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, *surface, NULL);
        *surface = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT)
{
    DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
}

void VulkanSystem::DestroyInstance(VkInstance* instance)
{
    if (*instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(*instance, NULL);
        *instance = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
{
    if (*renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, *renderPass, NULL);
        *renderPass = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyFrameBuffers(VkDevice device, Vector<VkFramebuffer>& frameBufferList)
{
    for (auto& frameBuffer : frameBufferList)
    {
        if (frameBuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
            frameBuffer = VK_NULL_HANDLE;
        }
    }
    frameBufferList.clear();
}

void VulkanSystem::DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
{
    if (*descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, *descriptorPool, NULL);
        *descriptorPool = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout)
{
    if (*descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, NULL);
        *descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, Vector<VkCommandBuffer>& commandBufferList)
{
    vkFreeCommandBuffers(device, *commandPool, commandBufferList.size(), commandBufferList.data());
    for (size_t x = 0; x < commandBufferList.size(); x++)
    {
        commandBufferList[x] = VK_NULL_HANDLE;
    }
    commandBufferList.clear();
}

void VulkanSystem::DestroyBuffer(VkDevice device, VkBuffer* buffer)
{
    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, *buffer, NULL);
        *buffer = VK_NULL_HANDLE;
    }
}

void VulkanSystem::FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory)
{
    if (*memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, *memory, NULL);
        *memory = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySwapChainImageView(VkDevice device, Vector<VkImageView>& swapChainImageViewList)
{
    for (auto& swapChainImageView : swapChainImageViewList)
    {
        if (swapChainImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, swapChainImageView, nullptr);
            swapChainImageView = VK_NULL_HANDLE;
        }
    }
    swapChainImageViewList.clear();
}

void VulkanSystem::DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain)
{
    vkDestroySwapchainKHR(device, *swapChain, NULL);
    *swapChain = VK_NULL_HANDLE;
}

void VulkanSystem::DestroyImageView(VkDevice device, VkImageView* imageView)
{
    if (*imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, *imageView, NULL);
        *imageView = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyImage(VkDevice device, VkImage* image)
{
    if (*image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, *image, NULL);
        *image = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySampler(VkDevice device, VkSampler* sampler)
{
    if (*sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, *sampler, NULL);
        *sampler = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipeline(VkDevice device, VkPipeline* pipeline)
{
    if (*pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, *pipeline, NULL);
        *pipeline = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout)
{
    if (*pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, *pipelineLayout, NULL);
        *pipelineLayout = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache)
{
    if (*pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, *pipelineCache, NULL);
        *pipelineCache = VK_NULL_HANDLE;
    }
}