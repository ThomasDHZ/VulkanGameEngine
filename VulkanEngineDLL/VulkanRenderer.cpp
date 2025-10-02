#define GLFW_INCLUDE_VULKAN
#include <glfw/include/GLFW/glfw3.h>
#include "VulkanRenderer.h"
#include <cstdlib>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "MemorySystem.h"

GraphicsRenderer renderer = GraphicsRenderer();

HWND editorRichTextBoxCallback;
LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;

void Debug_SetRichTextBoxHandle(HWND hwnd)
{
    editorRichTextBoxCallback = hwnd;
}

void SetLogVulkanMessageCallback(LogVulkanMessageCallback callback)
{
    g_logVulkanMessageCallback = callback;
}

void LogVulkanMessage(const char* message, int severity)
{
    if (g_logVulkanMessageCallback)
    {
        g_logVulkanMessageCallback(message, severity);
    }
}

GraphicsRenderer Renderer_RendererSetUp(WindowType windowType, void* windowHandle, GraphicsRenderer& renderer)
{
    renderer.ImageIndex = 0;
    renderer.CommandIndex = 0;
    renderer.InFlightFences = memorySystem.AddPtrBuffer<VkFence>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.AcquireImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.PresentImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.RebuildRendererFlag = false;
    renderer.PhysicalDevice = Renderer_SetUpPhysicalDevice(renderer.Instance, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily);
    renderer.Device = Renderer_SetUpDevice(renderer.PhysicalDevice, renderer.GraphicsFamily, renderer.PresentFamily);
    VULKAN_RESULT(Renderer_SetUpSwapChain(windowType, windowHandle, renderer));
    renderer.CommandPool = Renderer_SetUpCommandPool(renderer.Device, renderer.GraphicsFamily);
    VULKAN_RESULT(Renderer_SetUpSemaphores(renderer.Device, renderer.InFlightFences, renderer.AcquireImageSemaphores, renderer.PresentImageSemaphores, renderer.SwapChainImageCount));
    VULKAN_RESULT(Renderer_GetDeviceQueue(renderer.Device, renderer.GraphicsFamily, renderer.PresentFamily, renderer.GraphicsQueue, renderer.PresentQueue));

    return renderer;
}

GraphicsRenderer Renderer_RebuildSwapChain(WindowType windowType, void* windowHandle, GraphicsRenderer& renderer)
{
    vkDeviceWaitIdle(renderer.Device);
    Renderer_DestroySwapChainImageView(renderer.Device, renderer.Surface, &renderer.SwapChainImageViews[0], MAX_FRAMES_IN_FLIGHT);
    Renderer_DestroySwapChain(renderer.Device, &renderer.Swapchain);

    Renderer_SetUpSwapChain(windowType, windowHandle, renderer);
    return renderer;
}

VkResult Renderer_SetUpSwapChain(WindowType windowType, void* windowHandle, GraphicsRenderer& renderer)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = SwapChain_GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = SwapChain_GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    VULKAN_RESULT(SwapChain_GetQueueFamilies(renderer.PhysicalDevice, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily));
    Vector<VkPresentModeKHR> compatiblePresentModesList = SwapChain_GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = SwapChain_FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = SwapChain_FindSwapPresentMode(compatiblePresentModesList);

    SwapChain_SetUpSwapChain(renderer);
    renderer.SwapChainResolution.width = surfaceCapabilities.currentExtent.width;
    renderer.SwapChainResolution.height = surfaceCapabilities.currentExtent.height;
    renderer.SwapChainImages = SwapChain_SetUpSwapChainImages(renderer.Device, renderer.Swapchain, static_cast<uint32>(MAX_FRAMES_IN_FLIGHT));
    renderer.SwapChainImageViews = SwapChain_SetUpSwapChainImageViews(renderer.Device, renderer.SwapChainImages, renderer.SwapChainImageCount, swapChainImageFormat);
    return VK_SUCCESS;
}

VkCommandBuffer  Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    result = vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

    return result;
}

 void Renderer_DestroyRenderer(GraphicsRenderer& renderer)
 {
     Renderer_DestroySwapChainImageView(renderer.Device, renderer.Surface, &renderer.SwapChainImageViews[0], MAX_FRAMES_IN_FLIGHT);
     Renderer_DestroySwapChain(renderer.Device, &renderer.Swapchain);
     Renderer_DestroyFences(renderer.Device, &renderer.AcquireImageSemaphores[0], &renderer.PresentImageSemaphores[0], &renderer.InFlightFences[0], MAX_FRAMES_IN_FLIGHT);
     Renderer_DestroyCommandPool(renderer.Device, &renderer.CommandPool);
     Renderer_DestroyDevice(renderer.Device);
     Renderer_DestroyDebugger(&renderer.Instance, renderer.DebugMessenger);
     Renderer_DestroySurface(renderer.Instance, &renderer.Surface);
     Renderer_DestroyInstance(&renderer.Instance);

     memorySystem.RemovePtrBuffer(renderer.InFlightFences);
     memorySystem.RemovePtrBuffer(renderer.AcquireImageSemaphores);
     memorySystem.RemovePtrBuffer(renderer.PresentImageSemaphores);
     memorySystem.RemovePtrBuffer(renderer.SwapChainImages);
     memorySystem.RemovePtrBuffer(renderer.SwapChainImageViews);
 }

  VkSurfaceKHR Renderer_CreateVulkanSurface(void* windowHandle, VkInstance instance)
  {
      if (!windowHandle || !instance) {
          fprintf(stderr, "Invalid window handle (%p) or instance (%p)\n", windowHandle, instance);
          return VK_NULL_HANDLE;
      }

      VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
      surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      surfaceCreateInfo.pNext = nullptr;
      surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
      surfaceCreateInfo.hwnd = (HWND)windowHandle;

      VkSurfaceKHR surface;
      VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
      if (result != VK_SUCCESS) {
          fprintf(stderr, "Failed to create Vulkan surface: %d\n", result);
          return VK_NULL_HANDLE;
      }

      return surface;
  }

  VkBool32 VKAPI_CALL Vulkan_DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData)
  {
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole == INVALID_HANDLE_VALUE) {
          printf("Failed to get console handle.\n");
          return VK_FALSE;
      }
      CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
      GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
      WORD originalAttributes = consoleInfo.wAttributes;

      char message[4096];
      snprintf(message, sizeof(message), "%s", CallBackData->pMessage);

      switch (MessageSeverity)
      {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
          SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
          fprintf(stdout, "VERBOSE: ");
          SetConsoleTextAttribute(hConsole, originalAttributes);
          fprintf(stdout, "%s\n", message);
          break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
          SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
          fprintf(stdout, "INFO: ");
          SetConsoleTextAttribute(hConsole, originalAttributes);
          fprintf(stdout, "%s\n", message);
          break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
          SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
          fprintf(stdout, "WARNING: ");
          SetConsoleTextAttribute(hConsole, originalAttributes);
          fprintf(stdout, "%s\n", message);
          break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
          SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
          fprintf(stdout, "ERROR: ");
          SetConsoleTextAttribute(hConsole, originalAttributes);
          fprintf(stdout, "%s\n", message);
          break;
      default:
          SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
          fprintf(stderr, "UNKNOWN SEVERITY: ");
          SetConsoleTextAttribute(hConsole, originalAttributes);
          fprintf(stdout, "%s\n", message);
          break;
      }

      LogVulkanMessage(message, (int)MessageSeverity);
     
      if (editorRichTextBoxCallback)
      {
          char formattedMessage[4096];
          snprintf(formattedMessage, sizeof(formattedMessage), "%s\r\n", message);
          SendMessageA(editorRichTextBoxCallback, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
          SendMessageA(editorRichTextBoxCallback, EM_REPLACESEL, FALSE, (LPARAM)formattedMessage);
      }

      return VK_FALSE;
  }

Vector<VkExtensionProperties> Renderer_GetDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    Vector<VkExtensionProperties> deviceExtensionList = Vector<VkExtensionProperties>();

    uint32 deviceExtentionCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtentionCount, nullptr);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate device extension properties. Error: %d\n", result);
        return deviceExtensionList;
    }

    deviceExtensionList = Vector<VkExtensionProperties>(deviceExtentionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtentionCount, deviceExtensionList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate device extension properties. Error: %d\n", result);
        return deviceExtensionList;
    }

    return deviceExtensionList;
}

Vector<VkSurfaceFormatKHR> Renderer_GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    Vector<VkSurfaceFormatKHR> surfaceFormatList = Vector<VkSurfaceFormatKHR>();

    uint32 surfaceFormatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get the physical device surface formats count. Error: %d\n", result);
        return surfaceFormatList;
    }

    surfaceFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormatList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get physical device surface formats. Error: %d\n", result);
        return surfaceFormatList;
    }

    return surfaceFormatList;
}

Vector<VkPresentModeKHR> Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    Vector<VkPresentModeKHR> presentModeList = Vector<VkPresentModeKHR>();

    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get the physical device surface present modes count. Error: %d\n", result);
        return presentModeList;
    }

    presentModeList = Vector<VkPresentModeKHR>(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModeList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get physical device surface present modes. Error: %d\n", result);
        return presentModeList;
    }

    return presentModeList;
}

bool Renderer_GetRayTracingSupport()
{
    /* uint32 deviceExtensionCount = INT32_MAX;
     VkExtensionProperties* deviceExtensions = GetDeviceExtensions(cRenderer.PhysicalDevice, &deviceExtensionCount);
     VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures =
     {
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR
     };

     VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeatures =
     {
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
         .pNext = &physicalDeviceAccelerationStructureFeatures
     };

     VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
     {
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
         .pNext = &physicalDeviceRayTracingPipelineFeatures
     };
     vkGetPhysicalDeviceFeatures2(cRenderer.PhysicalDevice, &physicalDeviceFeatures2);

     if (physicalDeviceRayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
         physicalDeviceAccelerationStructureFeatures.accelerationStructure == VK_TRUE)
     {
         if (Array_RendererExtensionPropertiesSearch(deviceExtensions, deviceExtensionCount, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
             Array_RendererExtensionPropertiesSearch(deviceExtensions, deviceExtensionCount, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
         {
             uint32 pExtensionCount = 0;
             VkExtensionProperties* extensions = NULL;
             vulkanWindow->GetInstanceExtensions(vulkanWindow, &pExtensionCount, &extensions);
             return true;
         }
         else
         {
             fprintf(stderr, "GPU/Mother Board isn't ray tracing compatible.\n");
             free(deviceExtensions);
             return false;
         }
     }
     else
     {
         fprintf(stderr, "GPU/Mother Board isn't ray tracing compatible.\n");
         free(deviceExtensions);
         return false;
     }*/
    return false;
}

VkInstance Renderer_CreateVulkanInstance()
{
    VkInstance instance = VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = Vulkan_DebugCallBack;
   // debugInfo.pUserData = editorRichTextBoxCallback;

    VkValidationFeaturesEXT validationFeatures = {};
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.pNext = &debugInfo;
    validationFeatures.enabledValidationFeatureCount = static_cast<uint32_t>(enabledList.size());
    validationFeatures.pEnabledValidationFeatures = enabledList.data();
    validationFeatures.disabledValidationFeatureCount = static_cast<uint32_t>(disabledList.size());
    validationFeatures.pDisabledValidationFeatures = disabledList.data();

    int enableValidationLayers = 1;
#ifdef NDEBUG
    enableValidationLayers = 0;
#endif

    uint32_t extensionCount = 0;
    std::vector<std::string> extensionNames;
    std::vector<const char*> extensionNamesPointers;

    VkResult result = Renderer_GetWin32Extensions(&extensionCount, extensionNames);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get extensions: %d\n", result);
        return VK_NULL_HANDLE;
    }

    extensionNamesPointers.reserve(extensionCount);
    for (const auto& name : extensionNames) {
        extensionNamesPointers.push_back(name.c_str());
    }

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Application";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = enableValidationLayers ? &validationFeatures : nullptr;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = enableValidationLayers ? 1 : 0;
    createInfo.ppEnabledLayerNames = enableValidationLayers ? ValidationLayers : nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNamesPointers.size());
    createInfo.ppEnabledExtensionNames = extensionNamesPointers.data();

    result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance: %d\n", result);
        return VK_NULL_HANDLE;
    }

    return instance;
}

VkResult Renderer_GetWin32Extensions(uint32_t* extensionCount, std::vector<std::string>& enabledExtensions)
{
    if (!extensionCount) {
        fprintf(stderr, "Invalid argument: extensionCount cannot be NULL.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, extensionCount, nullptr);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to enumerate instance extension properties: %d\n", result);
        *extensionCount = 0;
        return result;
    }

    std::vector<VkExtensionProperties> extensions(*extensionCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, extensionCount, extensions.data());
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to enumerate instance extension properties after allocation: %d\n", result);
        *extensionCount = 0;
        return result;
    }

    enabledExtensions.clear();
    for (const char* requiredExt : InstanceExtensionList) {
        bool found = false;
        for (const auto& ext : extensions) {
            if (strcmp(ext.extensionName, requiredExt) == 0) {
                enabledExtensions.emplace_back(ext.extensionName);
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Required extension %s not supported\n", requiredExt);
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    *extensionCount = static_cast<uint32_t>(enabledExtensions.size());
    return VK_SUCCESS;
}

VkDebugUtilsMessengerEXT Renderer_SetupDebugMessenger(VkInstance instance)
{
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = Vulkan_DebugCallBack,
    };

    if (CreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to set up debug messenger!\n");
        return VK_NULL_HANDLE;
    }

    return debugMessenger;
}

VkPhysicalDeviceFeatures Renderer_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    return features;
}

Vector<VkPhysicalDevice> Renderer_GetPhysicalDeviceList(VkInstance& instance)
{
    Vector<VkPhysicalDevice> physicalDeviceList = Vector<VkPhysicalDevice>();

    uint32 deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate physical devices. Error: %d\n", result);
        return physicalDeviceList;
    }
    if (deviceCount == 0) {
        fprintf(stderr, "No physical devices found.\n");
        return physicalDeviceList;
    }

    physicalDeviceList = Vector<VkPhysicalDevice>(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate physical devices after allocation. Error: %d\n", result);
        return physicalDeviceList;
    }

    return physicalDeviceList;
}

VkPhysicalDevice Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    Vector<VkPhysicalDevice> physicalDeviceList = Renderer_GetPhysicalDeviceList(instance);
    for (auto& physicalDevice : physicalDeviceList)
    {
        VkPhysicalDeviceFeatures physicalDeviceFeatures = Renderer_GetPhysicalDeviceFeatures(physicalDevice);
        VkResult result = SwapChain_GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
        Vector<VkSurfaceFormatKHR> surfaceFormatList = Renderer_GetSurfaceFormats(physicalDevice, surface);
        Vector<VkPresentModeKHR> presentModeList = Renderer_GetSurfacePresentModes(physicalDevice, surface);

        if (graphicsFamily != UINT32_MAX &&
            presentFamily != UINT32_MAX &&
            surfaceFormatList.size() > 0 &&
            presentModeList.size() > 0 &&
            physicalDeviceFeatures.samplerAnisotropy)
        {
            return physicalDevice;
        }
    }

    return VK_NULL_HANDLE;
}

VkDevice Renderer_SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily)
{
    VkDevice device = VK_NULL_HANDLE;

    float queuePriority = 1.0f;
    Vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    if (graphicsFamily != UINT32_MAX)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = graphicsFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });
    }

    if (presentFamily != UINT32_MAX &&
        presentFamily != graphicsFamily)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo
            {
               .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
               .pNext = nullptr,
               .flags = 0,
               .queueFamilyIndex = presentFamily,
               .queueCount = 1,
               .pQueuePriorities = &queuePriority
            });
    }

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .pNext = nullptr,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
    };

    if (Renderer_GetRayTracingSupport())
    {
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .accelerationStructure = VK_TRUE,
            .accelerationStructureCaptureReplay = VK_FALSE,
            .accelerationStructureIndirectBuild = VK_FALSE,
            .accelerationStructureHostCommands = VK_FALSE,
            .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE,
        };

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = &accelerationStructureFeatures,
            .rayTracingPipeline = VK_TRUE,
            .rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE,
            .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
            .rayTracingPipelineTraceRaysIndirect = VK_FALSE,
            .rayTraversalPrimitiveCulling = VK_FALSE,
        };

        bufferDeviceAddressFeatures.pNext = &rayTracingPipelineFeatures;
    }
    else
    {
        bufferDeviceAddressFeatures.pNext = NULL;
    }

    VkPhysicalDeviceFeatures deviceFeatures =
    {
        .robustBufferAccess = VK_FALSE,
        .fullDrawIndexUint32 = VK_FALSE,
        .imageCubeArray = VK_FALSE,
        .independentBlend = VK_FALSE,
        .geometryShader = VK_FALSE,
        .tessellationShader = VK_FALSE,
        .sampleRateShading = VK_TRUE,
        .dualSrcBlend = VK_FALSE,
        .logicOp = VK_FALSE,
        .multiDrawIndirect = VK_FALSE,
        .drawIndirectFirstInstance = VK_FALSE,
        .depthClamp = VK_FALSE,
        .depthBiasClamp = VK_FALSE,
        .fillModeNonSolid = VK_TRUE,
        .depthBounds = VK_FALSE,
        .wideLines = VK_FALSE,
        .largePoints = VK_FALSE,
        .alphaToOne = VK_FALSE,
        .multiViewport = VK_FALSE,
        .samplerAnisotropy = VK_TRUE,
        .textureCompressionETC2 = VK_FALSE,
        .textureCompressionASTC_LDR = VK_FALSE,
        .textureCompressionBC = VK_FALSE,
        .occlusionQueryPrecise = VK_FALSE,
        .pipelineStatisticsQuery = VK_FALSE,
        .vertexPipelineStoresAndAtomics = VK_TRUE,
        .fragmentStoresAndAtomics = VK_TRUE,
        .shaderTessellationAndGeometryPointSize = VK_FALSE,
        .shaderImageGatherExtended = VK_FALSE,
        .shaderStorageImageExtendedFormats = VK_FALSE,
        .shaderStorageImageMultisample = VK_FALSE,
        .shaderStorageImageReadWithoutFormat = VK_FALSE,
        .shaderStorageImageWriteWithoutFormat = VK_FALSE,
        .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
        .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
        .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
        .shaderClipDistance = VK_FALSE,
        .shaderCullDistance = VK_FALSE,
        .shaderFloat64 = VK_FALSE,
        .shaderInt64 = VK_TRUE,
        .shaderInt16 = VK_FALSE,
        .shaderResourceResidency = VK_FALSE,
        .shaderResourceMinLod = VK_FALSE,
        .sparseBinding = VK_FALSE,
        .sparseResidencyBuffer = VK_FALSE,
        .sparseResidencyImage2D = VK_FALSE,
        .sparseResidencyImage3D = VK_FALSE,
        .sparseResidency2Samples = VK_FALSE,
        .sparseResidency4Samples = VK_FALSE,
        .sparseResidency8Samples = VK_FALSE,
        .sparseResidency16Samples = VK_FALSE,
        .sparseResidencyAliased = VK_FALSE,
        .variableMultisampleRate = VK_FALSE,
        .inheritedQueries = VK_FALSE,
    };

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = deviceFeatures
    };

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &physicalDeviceFeatures2,
        .samplerMirrorClampToEdge = VK_FALSE,
        .drawIndirectCount = VK_FALSE,
        .storageBuffer8BitAccess = VK_TRUE,
        .uniformAndStorageBuffer8BitAccess = VK_TRUE,
        .storagePushConstant8 = VK_FALSE,
        .shaderBufferInt64Atomics = VK_FALSE,
        .shaderSharedInt64Atomics = VK_FALSE,
        .shaderFloat16 = VK_FALSE,
        .shaderInt8 = VK_FALSE,
        .descriptorIndexing = VK_TRUE,
        .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
        .descriptorBindingPartiallyBound = VK_FALSE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .samplerFilterMinmax = VK_FALSE,
        .scalarBlockLayout = VK_FALSE,
        .imagelessFramebuffer = VK_FALSE,
        .uniformBufferStandardLayout = VK_FALSE,
        .shaderSubgroupExtendedTypes = VK_FALSE,
        .separateDepthStencilLayouts = VK_TRUE,
        .hostQueryReset = VK_FALSE,
        .timelineSemaphore = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
        .vulkanMemoryModel = VK_TRUE,
        .vulkanMemoryModelDeviceScope = VK_TRUE,
        .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
        .shaderOutputViewportIndex = VK_FALSE,
        .shaderOutputLayer = VK_FALSE,
        .subgroupBroadcastDynamicId = VK_FALSE,
    };

    VkPhysicalDeviceRobustness2FeaturesEXT physicalDeviceRobustness2Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan12Features,
        .robustBufferAccess2 = VK_FALSE,
        .robustImageAccess2 = VK_FALSE,
        .nullDescriptor = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &physicalDeviceRobustness2Features,
        .robustImageAccess = VK_FALSE,
        .inlineUniformBlock = VK_FALSE,
        .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
        .pipelineCreationCacheControl = VK_FALSE,
        .privateData = VK_FALSE,
        .shaderDemoteToHelperInvocation = VK_FALSE,
        .shaderTerminateInvocation = VK_FALSE,
        .subgroupSizeControl = VK_FALSE,
        .computeFullSubgroups = VK_FALSE,
        .synchronization2 = VK_FALSE,
        .textureCompressionASTC_HDR = VK_FALSE,
        .shaderZeroInitializeWorkgroupMemory = VK_FALSE,
        .dynamicRendering = VK_FALSE,
        .shaderIntegerDotProduct = VK_FALSE,
        .maintenance4 = VK_FALSE
    };

    VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT vertexAttributeRobustnessFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan13Features,
        .vertexAttributeRobustness = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vertexAttributeRobustnessFeatures,
        .multiview = VK_TRUE
    };

    VkDeviceCreateInfo deviceCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &physicalDeviceVulkan11Features,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfoList.size()),
        .pQueueCreateInfos = queueCreateInfoList.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = ARRAY_SIZE(DeviceExtensionList),
        .ppEnabledExtensionNames = DeviceExtensionList,
        .pEnabledFeatures = nullptr
    };

#ifdef NDEBUG
    deviceCreateInfo.enabledLayerCount = 0;
#else
    deviceCreateInfo.enabledLayerCount = ARRAY_SIZE(ValidationLayers);
    deviceCreateInfo.ppEnabledLayerNames = ValidationLayers;
#endif

    VULKAN_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device));
    return device;
}

VkCommandPool Renderer_SetUpCommandPool(VkDevice device, uint32 graphicsFamily)
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo CommandPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsFamily
    };
    VULKAN_RESULT(vkCreateCommandPool(device, &CommandPoolCreateInfo, NULL, &commandPool));
    return commandPool;
}


VkResult Renderer_GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue& graphicsQueue, VkQueue& presentQueue)
{
    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
    return VK_SUCCESS;
}

VkSurfaceFormatKHR SwapChain_FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (uint32_t x = 0; x < availableFormats.size(); x++)
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

VkPresentModeKHR SwapChain_FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
{
    for (uint32_t x = 0; x < availablePresentModes.size(); x++)
    {
        if (availablePresentModes[x] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentModes[x];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkResult SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    Vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());

    for (uint32 x = 0; x <= queueFamilyCount; x++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, surface, &presentSupport);

        if (queueFamilyList[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            presentFamily = x;
            graphicsFamily = x;
            break;
        }
    }

    return VK_SUCCESS;
}

VkSurfaceCapabilitiesKHR SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
    return surfaceCapabilities;
}

Vector<VkSurfaceFormatKHR> SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));

    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, compatibleSwapChainFormatList.data()));

    return compatibleSwapChainFormatList;
}

Vector<VkPresentModeKHR> SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 presentModeCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

    Vector<VkPresentModeKHR> compatiblePresentModesList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, compatiblePresentModesList.data()));

    return compatiblePresentModesList;
}

void SwapChain_SetUpSwapChain(GraphicsRenderer& renderer)
{
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = SwapChain_GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkPresentModeKHR> compatiblePresentModesList = SwapChain_GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);

    VkSurfaceCapabilitiesKHR surfaceCapabilities = SwapChain_GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = SwapChain_FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = SwapChain_FindSwapPresentMode(compatiblePresentModesList);

    renderer.SwapChainImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 &&
        renderer.SwapChainImageCount > surfaceCapabilities.maxImageCount)
    {
        renderer.SwapChainImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = renderer.Surface,
        .minImageCount = static_cast<uint32>(renderer.SwapChainImageCount),
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = surfaceCapabilities.maxImageExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = renderer.Swapchain
    };

    if (renderer.GraphicsFamily != renderer.PresentFamily)
    {
        Vector<uint32> queueFamilyIndices = 
        { 
            renderer.GraphicsFamily, 
            renderer.PresentFamily 
        };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_RESULT(vkCreateSwapchainKHR(renderer.Device, &SwapChainCreateInfo, nullptr, &renderer.Swapchain));
}

VkImage* SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount)
{
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr));

    VkImage* swapChainImageList = memorySystem.AddPtrBuffer<VkImage>(swapChainImageCount, __FILE__, __LINE__, __func__);
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImageList));

    return swapChainImageList;
}

VkImageView* SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat)
{
    VkImageView* imageViews = memorySystem.AddPtrBuffer<VkImageView>(swapChainImageCount, __FILE__, __LINE__, __func__);
    for (size_t x = 0; x < swapChainImageCount; x++)
    {
        VkImageViewCreateInfo swapChainViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImageList[x],
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
        VULKAN_RESULT(vkCreateImageView(device, &swapChainViewInfo, nullptr, &imageViews[x]));
    }

    return imageViews;
}

VkResult Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, size_t swapChainImageCount)
{
    return Renderer_CreateSemaphores(device, inFlightFences, acquireImageSemaphores, presentImageSemaphores, swapChainImageCount);
}