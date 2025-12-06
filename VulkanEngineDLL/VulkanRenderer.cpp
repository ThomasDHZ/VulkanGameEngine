#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanRenderer.h"
#include <cstdlib>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "MemorySystem.h"
#include "Platform.h"
#include "EngineConfigSystem.h"
#if defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

GraphicsRenderer renderer = GraphicsRenderer();
VulkanSystem vulkanSystem = VulkanSystem();

uint32 VulkanSystem::FindMaxApiVersion(VkPhysicalDevice physicalDevice)
{
	uint32 version = VulkanSystem::GetPhysicalDeviceProperties(physicalDevice).apiVersion;
#ifndef __ANDROID__
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 4)) return VK_API_VERSION_1_4;
#endif
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 3)) return VK_API_VERSION_1_3;
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 2)) return VK_API_VERSION_1_2;
    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 1)) return VK_API_VERSION_1_1;
}

GraphicsRenderer VulkanSystem::RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    renderer.ImageIndex = 0;
    renderer.CommandIndex = 0;
    renderer.RebuildRendererFlag = false;
    renderer.Instance = instance;
    renderer.Surface = surface;
    renderer.PhysicalDevice = VulkanSystem::SetUpPhysicalDevice(renderer.Instance, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily);
    renderer.Device = VulkanSystem::SetUpDevice(renderer.PhysicalDevice, renderer.GraphicsFamily, renderer.PresentFamily);

#if defined(__ANDROID__)
    renderer.vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(renderer.Device, "vkGetBufferDeviceAddress");
    if (renderer.vkGetBufferDeviceAddress == nullptr) {
        throw std::runtime_error("Failed to load vkGetBufferDeviceAddress function pointer!");
    }
#endif

    VulkanSystem::SetUpSwapChain(windowHandle);
    renderer.CommandPool = VulkanSystem::SetUpCommandPool(renderer.Device, renderer.GraphicsFamily);
    VulkanSystem::SetUpSemaphores(renderer.Device, renderer.InFlightFences, renderer.AcquireImageSemaphores, renderer.PresentImageSemaphores, renderer.SwapChainImageCount);
    VulkanSystem::GetDeviceQueue(renderer.Device, renderer.GraphicsFamily, renderer.PresentFamily, renderer.GraphicsQueue, renderer.PresentQueue);

    return renderer;
}

GraphicsRenderer VulkanSystem::RebuildSwapChain(void* windowHandle)
{
    vkDeviceWaitIdle(renderer.Device);
    VulkanSystem::DestroySwapChainImageView(renderer.Device, renderer.Surface, &renderer.SwapChainImageViews[0], renderer.SwapChainImageCount);
    VulkanSystem::DestroySwapChain(renderer.Device, &renderer.Swapchain);

    VulkanSystem::SetUpSwapChain(windowHandle);
    return renderer;
}

VkExtent2D VulkanSystem::SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
#ifndef PLATFORM_ANDROID
    int width;
    int height;
    glfwGetFramebufferSize((GLFWwindow*)windowHandle, &width, &height);

    surfaceCapabilities = VulkanSystem::GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return surfaceCapabilities.currentExtent;
    }

    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
#else
    VkExtent2D extent = {
            surfaceCapabilities.currentExtent.width,
            surfaceCapabilities.currentExtent.height
    };
    if (extent.width == UINT32_MAX) {
        extent = { 1280, 720 };
    }
#endif
    extent.width  = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
    return extent;
}

void VulkanSystem::SetUpSwapChain(void* windowHandle)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = VulkanSystem::GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    VkExtent2D extent = VulkanSystem::SetUpSwapChainExtent(windowHandle, surfaceCapabilities);
    VulkanSystem::GetQueueFamilies(renderer.PhysicalDevice, renderer.Surface, renderer.GraphicsFamily, renderer.PresentFamily);
    Vector<VkPresentModeKHR> compatiblePresentModesList = VulkanSystem::GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = VulkanSystem::FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = VulkanSystem::FindSwapPresentMode(compatiblePresentModesList);

    VulkanSystem::SetUpSwapChain();
    renderer.SwapChainImages = VulkanSystem::SetUpSwapChainImages(renderer.Device, renderer.Swapchain, static_cast<uint32>(renderer.SwapChainImageCount));
    renderer.SwapChainImageViews = VulkanSystem::SetUpSwapChainImageViews(renderer.Device, renderer.SwapChainImages, renderer.SwapChainImageCount, swapChainImageFormat);
    renderer.InFlightFences = memorySystem.AddPtrBuffer<VkFence>(renderer.SwapChainImageCount, __FILE__, __LINE__, __func__);
    renderer.AcquireImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(renderer.SwapChainImageCount, __FILE__, __LINE__, __func__);
    renderer.PresentImageSemaphores = memorySystem.AddPtrBuffer<VkSemaphore>(renderer.SwapChainImageCount, __FILE__, __LINE__, __func__);
}

 void VulkanSystem::DestroyRenderer(GraphicsRenderer& renderer)
 {
     VulkanSystem::DestroySwapChainImageView(renderer.Device, renderer.Surface, &renderer.SwapChainImageViews[0], renderer.SwapChainImageCount);
     VulkanSystem::DestroySwapChain(renderer.Device, &renderer.Swapchain);
     VulkanSystem::DestroyFences(renderer.Device, &renderer.AcquireImageSemaphores[0], &renderer.PresentImageSemaphores[0], &renderer.InFlightFences[0], renderer.SwapChainImageCount);
     VulkanSystem::DestroyCommandPool(renderer.Device, &renderer.CommandPool);
     VulkanSystem::DestroyDevice(renderer.Device);
     VulkanSystem::DestroyDebugger(&renderer.Instance, renderer.DebugMessenger);
     VulkanSystem::DestroySurface(renderer.Instance, &renderer.Surface);
     VulkanSystem::DestroyInstance(&renderer.Instance);

     memorySystem.DeletePtr(renderer.InFlightFences);
     memorySystem.DeletePtr(renderer.AcquireImageSemaphores);
     memorySystem.DeletePtr(renderer.PresentImageSemaphores);
     memorySystem.DeletePtr(renderer.SwapChainImages);
     memorySystem.DeletePtr(renderer.SwapChainImageViews);
 }

  VkSurfaceKHR VulkanSystem::CreateVulkanSurface(void* windowHandle, VkInstance instance)
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
      Vector<VkPresentModeKHR>(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));

#elif defined(__linux__) && !defined(__ANDROID__)
      GLFWwindow* window = (GLFWwindow*)windowHandle;
      glfwCreateWindowSurface(instance, window, nullptr, &surface);

#elif defined(__ANDROID__)
      VkAndroidSurfaceCreateInfoKHR surfaceInfo =
      {
              .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
              .pNext = nullptr,
              .window = (ANativeWindow*)windowHandle
      };

      if (vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
      {
          __android_log_print(ANDROID_LOG_ERROR, "Vulkan", "Failed to create Android surface!");
          return VK_NULL_HANDLE;
      }
#endif

      return surface;
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

VkInstance VulkanSystem::CreateVulkanInstance()
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    Vector<VkValidationFeatureEnableEXT> enabledList;
    Vector<VkValidationFeatureDisableEXT> disabledList;

#ifndef NDEBUG
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

    /* debugInfo =
     {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanSystem::DebugCallBack
     };*/
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

    Vector<const char*> extensionNames = VulkanSystem::GetRequiredInstanceExtensions();
    VkApplicationInfo applicationInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        #if defined(__ANDROID__)
            .apiVersion = VK_API_VERSION_1_3
        #else
            .apiVersion = VK_API_VERSION_1_4
        #endif
    };


    Vector<const char*> validationLayers = VulkanSystem::GetValidationLayerProperties();
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
    VULKAN_THROW_IF_FAIL(vkCreateInstance(&createInfo, nullptr, &instance));

#ifndef NDEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        func(instance, &debugInfo, nullptr, &renderer.DebugMessenger);
    }
#endif

    return instance;
}

VkPhysicalDeviceFeatures VulkanSystem::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
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

VkPhysicalDevice VulkanSystem::SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    Vector<VkPhysicalDevice> physicalDeviceList = VulkanSystem::GetPhysicalDeviceList(instance);
    for (auto& physicalDevice : physicalDeviceList)
    {
		VkPhysicalDeviceProperties physicalDeviceProperties = VulkanSystem::GetPhysicalDeviceProperties(physicalDevice);
        VkPhysicalDeviceFeatures physicalDeviceFeatures = VulkanSystem::GetPhysicalDeviceFeatures(physicalDevice);
        VulkanSystem::GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
        Vector<VkSurfaceFormatKHR> surfaceFormatList = VulkanSystem::GetSurfaceFormats(physicalDevice, surface);
        Vector<VkPresentModeKHR> presentModeList = VulkanSystem::GetSurfacePresentModes(physicalDevice, surface);

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

VkDevice VulkanSystem::SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily)
{
    float queuePriority = 1.0f;
    VkDevice device = VK_NULL_HANDLE;
    Vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    if (graphicsFamily != UINT32_MAX)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }
    if (presentFamily != UINT32_MAX && presentFamily != graphicsFamily)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = presentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }

    Vector<const char*> DeviceExtensionList = VulkanSystem::GetRequiredDeviceExtensions(physicalDevice);
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

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = deviceFeatures
    };

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features = {
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

    VkPhysicalDeviceRobustness2FeaturesEXT physicalDeviceRobustness2Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan12Features,
        .nullDescriptor = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features = {
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

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT divisorFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan13Features,
        .vertexAttributeInstanceRateDivisor = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &divisorFeatures,
        .multiview = VK_TRUE
    };

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .pNext = &physicalDeviceVulkan11Features,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &bufferDeviceAddressFeatures,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfoList.size()),
        .pQueueCreateInfos = queueCreateInfoList.data(),
        .enabledExtensionCount = static_cast<uint32>(DeviceExtensionList.size()),
        .ppEnabledExtensionNames = DeviceExtensionList.data(),
        .pEnabledFeatures = nullptr  
    };

#ifndef NDEBUG
	Vector<const char*> validationLayers = VulkanSystem::GetValidationLayerProperties();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    VULKAN_THROW_IF_FAIL(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
    return device;
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
    VkSurfaceCapabilitiesKHR surfaceCapabilities = VulkanSystem::GetSurfaceCapabilities(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = VulkanSystem::GetPhysicalDeviceFormats(renderer.PhysicalDevice, renderer.Surface);
    Vector<VkPresentModeKHR> compatiblePresentModesList = VulkanSystem::GetPhysicalDevicePresentModes(renderer.PhysicalDevice, renderer.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = VulkanSystem::FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = VulkanSystem::FindSwapPresentMode(compatiblePresentModesList);

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

    renderer.SwapChainResolution = extent;
    renderer.SwapChainImageCount = imageCount;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = renderer.Surface,
        .minImageCount = static_cast<uint32>(renderer.SwapChainImageCount),
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = extent,
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
    VULKAN_THROW_IF_FAIL(vkCreateSwapchainKHR(renderer.Device, &SwapChainCreateInfo, nullptr, &renderer.Swapchain));
}

VkImage* VulkanSystem::SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount)
{
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr));
    VkImage* swapChainImageList = memorySystem.AddPtrBuffer<VkImage>(swapChainImageCount, __FILE__, __LINE__, __func__);
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImageList));
    return swapChainImageList;
}

VkImageView* VulkanSystem::SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat)
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
        VULKAN_THROW_IF_FAIL(vkCreateImageView(device, &swapChainViewInfo, nullptr, &imageViews[x]));
    }

    return imageViews;
}


void VulkanSystem::SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight)
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
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &acquireImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &presentImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[x]));
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

VkCommandBuffer VulkanSystem::BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

void VulkanSystem::EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));
    VULKAN_THROW_IF_FAIL(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_THROW_IF_FAIL(vkQueueWaitIdle(graphicsQueue));
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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

void VulkanSystem::DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount)
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
    VulkanSystem::DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
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

void VulkanSystem::DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count)
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

void VulkanSystem::DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count)
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

VulkanSystem::VulkanSystem()
{
}

VulkanSystem::~VulkanSystem()
{
}

void VulkanSystem::DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count)
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