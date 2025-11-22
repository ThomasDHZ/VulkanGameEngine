#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanRenderer.h"
#include <cstdlib>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "MemorySystem.h"
#include "EngineConfigSystem.h"

LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;
GraphicsRenderer renderer = GraphicsRenderer();

void Renderer_CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
    g_logVulkanMessageCallback = callback;
}

void Renderer_LogVulkanMessage(const char* message, int severity)
{
    if (g_logVulkanMessageCallback)
    {
        g_logVulkanMessageCallback(message, severity);
    }
}

GraphicsRenderer Renderer_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    renderer.ImageIndex = 0;
    renderer.CommandIndex = 0;
    renderer.InFlightFences = memorySystem.AddPtrBuffer<VkFence>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.AcquireImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.PresentImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(MAX_FRAMES_IN_FLIGHT, __FILE__, __LINE__, __func__);
    renderer.RebuildRendererFlag = false;
    renderer.Instance = instance;
    renderer.Surface = surface;
    renderer.PhysicalDevice = Renderer_SetUpPhysicalDevice(renderer.Instance, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily);
    renderer.Device = Renderer_SetUpDevice(renderer.PhysicalDevice, renderer.GraphicsFamily, renderer.PresentFamily);
    VULKAN_RESULT(Renderer_SetUpSwapChain(windowHandle, renderer));
    renderer.CommandPool = Renderer_SetUpCommandPool(renderer.Device, renderer.GraphicsFamily);
    VULKAN_RESULT(Renderer_SetUpSemaphores(renderer.Device, renderer.InFlightFences, renderer.AcquireImageSemaphores, renderer.PresentImageSemaphores, renderer.SwapChainImageCount));
    VULKAN_RESULT(Renderer_GetDeviceQueue(renderer.Device, renderer.GraphicsFamily, renderer.PresentFamily, renderer.GraphicsQueue, renderer.PresentQueue));

    return renderer;
}

GraphicsRenderer Renderer_RebuildSwapChain(void* windowHandle, GraphicsRenderer& renderer)
{
    vkDeviceWaitIdle(renderer.Device);
    Renderer_DestroySwapChainImageView(renderer.Device, renderer.Surface, &renderer.SwapChainImageViews[0], MAX_FRAMES_IN_FLIGHT);
    Renderer_DestroySwapChain(renderer.Device, &renderer.Swapchain);

    Renderer_SetUpSwapChain(windowHandle, renderer);
    return renderer;
}

VkResult Renderer_SetUpSwapChain(void* windowHandle, GraphicsRenderer& renderer)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = Renderer_GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Renderer_GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    VULKAN_RESULT(Renderer_GetQueueFamilies(renderer.PhysicalDevice, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily));
    Vector<VkPresentModeKHR> compatiblePresentModesList = Renderer_GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = Renderer_FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = Renderer_FindSwapPresentMode(compatiblePresentModesList);

    Renderer_SetUpSwapChain(renderer);
    renderer.SwapChainResolution.width = surfaceCapabilities.currentExtent.width;
    renderer.SwapChainResolution.height = surfaceCapabilities.currentExtent.height;
    renderer.SwapChainImages = Renderer_SetUpSwapChainImages(renderer.Device, renderer.Swapchain, static_cast<uint32>(MAX_FRAMES_IN_FLIGHT));
    renderer.SwapChainImageViews = Renderer_SetUpSwapChainImageViews(renderer.Device, renderer.SwapChainImages, renderer.SwapChainImageCount, swapChainImageFormat);
    return VK_SUCCESS;
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

     memorySystem.DeletePtr(renderer.InFlightFences);
     memorySystem.DeletePtr(renderer.AcquireImageSemaphores);
     memorySystem.DeletePtr(renderer.PresentImageSemaphores);
     memorySystem.DeletePtr(renderer.SwapChainImages);
     memorySystem.DeletePtr(renderer.SwapChainImageViews);
 }

  VkSurfaceKHR Renderer_CreateVulkanSurface(void* windowHandle, VkInstance instance)
  {
      if (!windowHandle || !instance) 
      {
          fprintf(stderr, "Invalid window handle (%p) or instance (%p)\n", windowHandle, instance);
          return VK_NULL_HANDLE;
      }

      VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(_WIN32)
      VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
      {
          .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
          .pNext = nullptr,
          .hinstance = GetModuleHandle(nullptr),
          .hwnd = (HWND)windowHandle
      };
      VULKAN_RESULT(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));

#elif defined(__linux__) && !defined(__ANDROID__)
      GLFWwindow* window = (GLFWwindow*)windowHandle;
      VULKAN_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));

#elif defined(__ANDROID__)
      return VK_NULL_HANDLE;
#endif

      return surface;
  }

  Vector<const char*> Renderer_GetRequiredInstanceExtensions()
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

#if defined(_WIN32)
      AddExtensionIfSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#elif defined(__linux__) && !defined(__ANDROID__)
      AddExtensionIfSupported("VK_KHR_xcb_surface");
      AddExtensionIfSupported("VK_KHR_wayland_surface");

#elif defined(__ANDROID__)
      AddExtenstionIfSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#ifndef NDEBUG
      AddExtensionIfSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

      if (extensions.empty() ||
          (extensions.size() == 1 && extensions[0] == VK_KHR_SURFACE_EXTENSION_NAME)) 
      {
          throw std::runtime_error("No platform surface extension available — cannot create window.");
      }
      return extensions;
  }

  Vector<const char*> Renderer_GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
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
      return enabledDeviceExtensions;
  }

  VkBool32 VKAPI_CALL Renderer_DebugCallBack(
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

      if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &consoleInfo))
          originalAttributes = consoleInfo.wAttributes;

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
      return VK_FALSE;
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

VkInstance Renderer_CreateVulkanInstance()
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    Vector<const char*> validationLayers;
    Vector<VkValidationFeatureEnableEXT> enabledList;
    Vector<VkValidationFeatureDisableEXT> disabledList;

#ifndef NDEBUG
     validationLayers = { "VK_LAYER_KHRONOS_validation" };

     enabledList =
     {
         VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
         VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
         VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
         VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
     };

     disabledList =
     {
         VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
         VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
         VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT
     };

     debugInfo =
     {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = Renderer_DebugCallBack
     };
#endif

     VkValidationFeaturesEXT validationFeatures = {
           .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
           .pNext = nullptr,
           .enabledValidationFeatureCount = static_cast<uint32_t>(enabledList.size()),
           .pEnabledValidationFeatures = enabledList.data(),
           .disabledValidationFeatureCount = static_cast<uint32_t>(disabledList.size()),
           .pDisabledValidationFeatures = disabledList.data()
     };

#ifndef NDEBUG
     validationFeatures.pNext = &debugInfo;
#endif

    Vector<const char*> extensionNames = Renderer_GetRequiredInstanceExtensions();
    VkApplicationInfo applicationInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4
    };

    VkInstanceCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &validationFeatures,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensionNames.size()),
        .ppEnabledExtensionNames = extensionNames.data()
    };
    
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance: %d\n", result);
        return VK_NULL_HANDLE;
    }

#ifndef NDEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        func(instance, &debugInfo, nullptr, &renderer.DebugMessenger);
    }
#endif

    return instance;
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
        VkResult result = Renderer_GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
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

    //if (Renderer_GetRayTracingSupport())
    //{
    //    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures =
    //    {
    //        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
    //        .accelerationStructure = VK_TRUE,
    //        .accelerationStructureCaptureReplay = VK_FALSE,
    //        .accelerationStructureIndirectBuild = VK_FALSE,
    //        .accelerationStructureHostCommands = VK_FALSE,
    //        .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE,
    //    };

    //    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures =
    //    {
    //        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
    //        .pNext = &accelerationStructureFeatures,
    //        .rayTracingPipeline = VK_TRUE,
    //        .rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE,
    //        .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
    //        .rayTracingPipelineTraceRaysIndirect = VK_FALSE,
    //        .rayTraversalPrimitiveCulling = VK_FALSE,
    //    };

    //    bufferDeviceAddressFeatures.pNext = &rayTracingPipelineFeatures;
    //}
    //else
    //{
    //    bufferDeviceAddressFeatures.pNext = NULL;
    //}

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
        .scalarBlockLayout = VK_TRUE,
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

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT divisorFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan13Features,
        .vertexAttributeInstanceRateDivisor = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &divisorFeatures,
        .multiview = VK_TRUE
    };

    Vector<const char*> DeviceExtensionList = Renderer_GetRequiredDeviceExtensions(physicalDevice);
    VkDeviceCreateInfo deviceCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &physicalDeviceVulkan11Features,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfoList.size()),
        .pQueueCreateInfos = queueCreateInfoList.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32>(DeviceExtensionList.size()),
        .ppEnabledExtensionNames = DeviceExtensionList.data(),
        .pEnabledFeatures = nullptr
    };

#ifndef NDEBUG
    Vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    deviceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
    deviceCreateInfo.enabledLayerCount = 0;
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

VkSurfaceFormatKHR Renderer_FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR Renderer_FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
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

VkResult Renderer_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
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
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, surface, &presentSupport);
            if (presentSupport) presentFamily = x;
            if (graphicsFamily != UINT32_MAX) break;
        }
    }

    return VK_SUCCESS;
}

VkSurfaceCapabilitiesKHR Renderer_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
    return surfaceCapabilities;
}

Vector<VkSurfaceFormatKHR> Renderer_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, compatibleSwapChainFormatList.data()));
    return compatibleSwapChainFormatList;
}

Vector<VkPresentModeKHR> Renderer_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 presentModeCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    Vector<VkPresentModeKHR> compatiblePresentModesList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, compatiblePresentModesList.data()));
    return compatiblePresentModesList;
}

void Renderer_SetUpSwapChain(GraphicsRenderer& renderer)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = Renderer_GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Renderer_GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkPresentModeKHR> compatiblePresentModesList = Renderer_GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = Renderer_FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = Renderer_FindSwapPresentMode(compatiblePresentModesList);

    renderer.SwapChainImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 &&
        renderer.SwapChainImageCount > surfaceCapabilities.maxImageCount)
    {
        renderer.SwapChainImageCount = surfaceCapabilities.maxImageCount;
    }

    VkExtent2D extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX) {
        // Window manager allows free choice (rare on Linux, common on Android)
        extent.width = std::clamp(static_cast<uint32>(configSystem.WindowResolution.x), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32>(configSystem.WindowResolution.y), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && 
        imageCount > surfaceCapabilities.maxImageCount)
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    renderer.SwapChainResolution = extent;
    renderer.SwapChainImageCount = imageCount;

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

VkImage* Renderer_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount)
{
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr));

    VkImage* swapChainImageList = memorySystem.AddPtrBuffer<VkImage>(swapChainImageCount, __FILE__, __LINE__, __func__);
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImageList));

    return swapChainImageList;
}

VkImageView* Renderer_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat)
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


VkResult Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight)
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

    for (int x = 0; x < maxFramesInFlight; x++)
    {
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &acquireImageSemaphores[x]));
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &presentImageSemaphores[x]));
        VULKAN_RESULT(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[x]));
    }

    return VK_SUCCESS;
}

uint32_t Renderer_GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkCommandBuffer Renderer_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

VkResult Renderer_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    VULKAN_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_RESULT(vkQueueWaitIdle(graphicsQueue));
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return VK_SUCCESS;
}

void Renderer_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator)
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

void Renderer_DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount)
{
    for (size_t x = 0; x < semaphoreCount; x++)
    {
        if (acquireImageSemaphores[x] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, acquireImageSemaphores[x], NULL);
            acquireImageSemaphores[x] = VK_NULL_HANDLE;
        }
        if (presentImageSemaphores[x] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, presentImageSemaphores[x], NULL);
            presentImageSemaphores[x] = VK_NULL_HANDLE;
        }
        if (fences[x] != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fences[x], NULL);
            fences[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyCommandPool(VkDevice device, VkCommandPool* commandPool)
{
    if (*commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, *commandPool, NULL);
        *commandPool = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDevice(VkDevice device)
{
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, NULL);
        device = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (*surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, *surface, NULL);
        *surface = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT)
{
    Renderer_DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
}

void Renderer_DestroyInstance(VkInstance* instance)
{
    if (*instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(*instance, NULL);
        *instance = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
{
    if (*renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, *renderPass, NULL);
        *renderPass = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count)
{
    for (size_t x = 0; x < count; x++)
    {
        if (frameBufferList[x] != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, frameBufferList[x], NULL);
            frameBufferList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
{
    if (*descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, *descriptorPool, NULL);
        *descriptorPool = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout)
{
    if (*descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, NULL);
        *descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count)
{
    if (*commandBufferList != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device, *commandPool, count, &*commandBufferList);
        for (size_t x = 0; x < count; x++)
        {
            commandBufferList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyBuffer(VkDevice device, VkBuffer* buffer)
{
    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, *buffer, NULL);
        *buffer = VK_NULL_HANDLE;
    }
}

void Renderer_FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory)
{
    if (*memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, *memory, NULL);
        *memory = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count)
{
    for (uint32 x = 0; x < count; x++)
    {
        if (surface != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, pSwapChainImageViewList[x], NULL);
            pSwapChainImageViewList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain)
{
    vkDestroySwapchainKHR(device, *swapChain, NULL);
    *swapChain = VK_NULL_HANDLE;
}

void Renderer_DestroyImageView(VkDevice device, VkImageView* imageView)
{
    if (*imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, *imageView, NULL);
        *imageView = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyImage(VkDevice device, VkImage* image)
{
    if (*image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, *image, NULL);
        *image = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySampler(VkDevice device, VkSampler* sampler)
{
    if (*sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, *sampler, NULL);
        *sampler = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipeline(VkDevice device, VkPipeline* pipeline)
{
    if (*pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, *pipeline, NULL);
        *pipeline = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout)
{
    if (*pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, *pipelineLayout, NULL);
        *pipelineLayout = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache)
{
    if (*pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, *pipelineCache, NULL);
        *pipelineCache = VK_NULL_HANDLE;
    }
}