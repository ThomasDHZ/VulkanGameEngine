#include "CoreVulkanRenderer.h"
#include <cstdlib>
#include <iostream>

RendererState cRenderer = { 0 };

List<VkExtensionProperties> Renderer_GetDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    List<VkExtensionProperties> deviceExtensionList = List<VkExtensionProperties>();

    uint32 deviceExtentionCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtentionCount, nullptr);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate device extension properties. Error: %d\n", result);
        return deviceExtensionList;
    }

    deviceExtensionList = List<VkExtensionProperties>(deviceExtentionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtentionCount, deviceExtensionList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate device extension properties. Error: %d\n", result);
        return deviceExtensionList;
    }

    return deviceExtensionList;
}

List<VkSurfaceFormatKHR> Renderer_GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    List<VkSurfaceFormatKHR> surfaceFormatList = List<VkSurfaceFormatKHR>();

    uint32 surfaceFormatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get the physical device surface formats count. Error: %d\n", result);
        return surfaceFormatList;
    }

    surfaceFormatList = List<VkSurfaceFormatKHR>(surfaceFormatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormatList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get physical device surface formats. Error: %d\n", result);
        return surfaceFormatList;
    }

    return surfaceFormatList;
}

List<VkPresentModeKHR> Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    List<VkPresentModeKHR> presentModeList = List<VkPresentModeKHR>();

    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to get the physical device surface present modes count. Error: %d\n", result);
        return presentModeList;
    }

    presentModeList = List<VkPresentModeKHR>(presentModeCount);
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

void Renderer_GetRendererFeatures(VkPhysicalDeviceVulkan11Features* physicalDeviceVulkan11Features)
{
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .pNext = nullptr,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
    };

    VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,
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
        .runtimeDescriptorArray = VK_FALSE,
    };

    VkPhysicalDeviceRobustness2FeaturesEXT PhysicalDeviceRobustness2Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceDescriptorIndexingFeatures,
        .robustBufferAccess2 = VK_FALSE,
        .robustImageAccess2 = VK_FALSE,
        .nullDescriptor = VK_FALSE
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
    PhysicalDeviceRobustness2Features.pNext = &bufferDeviceAddressFeatures;

    VkPhysicalDeviceFeatures deviceFeatures =
    {
        .robustBufferAccess = VK_FALSE,
        .fullDrawIndexUint32 = VK_FALSE,
        .imageCubeArray = VK_FALSE,
        .independentBlend = VK_FALSE,
        .geometryShader = VK_FALSE,
        .tessellationShader = VK_FALSE,
        .sampleRateShading = VK_FALSE,
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
        .vertexPipelineStoresAndAtomics = VK_FALSE,
        .fragmentStoresAndAtomics = VK_FALSE,
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
        .shaderInt64 = VK_FALSE,
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

    VkPhysicalDeviceFeatures2 deviceFeatures2 =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &PhysicalDeviceRobustness2Features,
        .features = deviceFeatures,
    };

    physicalDeviceVulkan11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    physicalDeviceVulkan11Features->multiview = VK_TRUE;
    physicalDeviceVulkan11Features->pNext = &deviceFeatures2;
}

VkInstance Renderer_CreateVulkanInstance()
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT* debugMessenger = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = Vulkan_DebugCallBack
    };

    int enableValidationLayers = 1;
#ifdef NDEBUG
    enableValidationLayers = 0;
#endif

    uint32_t extensionCount = 0;
    char** enabledExtensions = nullptr;
        Renderer_GetWin32Extensions(&extensionCount, &enabledExtensions);

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo vulkanCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32>(enableValidationLayers),
        .ppEnabledLayerNames = (enableValidationLayers ? ValidationLayers : nullptr),
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = enabledExtensions
    };
    if (enableValidationLayers)
    {
        vulkanCreateInfo.pNext = &debugInfo;
    }

    VkResult result = vkCreateInstance(&vulkanCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS) 
    {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        return VK_NULL_HANDLE;
    }

    return instance;
}

VkDebugUtilsMessengerEXT Renderer_SetupDebugMessenger(VkInstance instance)
{
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = Vulkan_DebugCallBack
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

List<VkPhysicalDevice> Renderer_GetPhysicalDeviceList(VkInstance& instance)
{
    List<VkPhysicalDevice> physicalDeviceList = List<VkPhysicalDevice>();

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

    physicalDeviceList = List<VkPhysicalDevice>(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data());
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to enumerate physical devices after allocation. Error: %d\n", result);
        return physicalDeviceList;
    }

    return physicalDeviceList;
}

VkPhysicalDevice Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32 graphicsFamily, uint32 presentFamily)
{
    List<VkPhysicalDevice> physicalDeviceList = Renderer_GetPhysicalDeviceList(instance);
    for (auto& physicalDevice : physicalDeviceList)
    {
        VkPhysicalDeviceFeatures physicalDeviceFeatures = Renderer_GetPhysicalDeviceFeatures(physicalDevice);
        VkResult result = SwapChain_GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
        List<VkSurfaceFormatKHR> surfaceFormatList = Renderer_GetSurfaceFormats(physicalDevice, surface);
        List<VkPresentModeKHR> presentModeList = Renderer_GetSurfacePresentModes(physicalDevice, surface);

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
    List<VkDeviceQueueCreateInfo> queueCreateInfoList;
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

    VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = &bufferDeviceAddressFeatures,
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
    };

    VkPhysicalDeviceRobustness2FeaturesEXT physicalDeviceRobustness2Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceDescriptorIndexingFeatures,
        .robustBufferAccess2 = VK_FALSE,
        .robustImageAccess2 = VK_FALSE,
        .nullDescriptor = VK_FALSE
    };

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
        .shaderInt64 = VK_FALSE,
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

    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features = 
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = nullptr,
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

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = 
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &physicalDeviceVulkan13Features,
        .features = deviceFeatures
    };
    
    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &physicalDeviceFeatures2,
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

VkSurfaceFormatKHR SwapChain_FindSwapSurfaceFormat(List<VkSurfaceFormatKHR>& availableFormats)
{
    for (uint32_t x = 0; x < availableFormats.size(); x++)
    {
        if (availableFormats[x].format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormats[x].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormats[x];
        }
    }
    fprintf(stderr, "Couldn't find a usable swap surface format.\n");
    return VkSurfaceFormatKHR { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
}

VkPresentModeKHR SwapChain_FindSwapPresentMode(List<VkPresentModeKHR>& availablePresentModes)
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

    List<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    return surfaceCapabilities;
}

List<VkSurfaceFormatKHR> SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));

    List<VkSurfaceFormatKHR> compatibleSwapChainFormatList = List<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, compatibleSwapChainFormatList.data()));

    return compatibleSwapChainFormatList;
}

List<VkPresentModeKHR> SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 presentModeCount = 0;
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));

    List<VkPresentModeKHR> compatiblePresentModesList = List<VkPresentModeKHR>(presentModeCount);
    VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, compatiblePresentModesList.data()));

    return compatiblePresentModesList;
}

VkSwapchainKHR SwapChain_SetUpSwapChain(VkDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR surfaceCapabilities, VkSurfaceFormatKHR SwapChainImageFormat, VkPresentModeKHR SwapChainPresentMode, uint32 graphicsFamily, uint32 presentFamily, uint32 width, uint32 height, uint32* swapChainImageCount)
{
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    VkExtent2D extent =
    {
        width,
        height
    };

    *swapChainImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 &&
        *swapChainImageCount > surfaceCapabilities.maxImageCount)
    {
        *swapChainImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = *swapChainImageCount,
        .imageFormat = SwapChainImageFormat.format,
        .imageColorSpace = SwapChainImageFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = SwapChainPresentMode,
        .clipped = VK_TRUE
    };

    if (graphicsFamily != presentFamily)
    {
        List<uint32> queueFamilyIndices = { graphicsFamily, presentFamily };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_RESULT(vkCreateSwapchainKHR(device, &SwapChainCreateInfo, nullptr, &swapChain));
    return swapChain;
}

List<VkImage> SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain)
{
    List<VkImage> swapChainImageList;

    uint32 swapChainImageCount = MAX_FRAMES_IN_FLIGHT;
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr));

    swapChainImageList = List<VkImage>(swapChainImageCount);
    VULKAN_RESULT(vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImageList.data()));

    return swapChainImageList;
}

List<VkImageView> SwapChain_SetUpSwapChainImageViews(VkDevice device, List<VkImage> swapChainImageList, VkSurfaceFormatKHR& swapChainImageFormat)
{
    std::vector<VkImageView> swapChainImageViewList(swapChainImageList.size());
    for (uint32_t x = 0; x < swapChainImageList.size(); x++)
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
        VULKAN_RESULT(vkCreateImageView(device, &swapChainViewInfo, nullptr, &swapChainImageViewList[x]));
    }

    return swapChainImageViewList;
}

VkResult Renderer_SetUpSemaphores(VkDevice device, List<VkFence>& inFlightFences, List<VkSemaphore>& acquireImageSemaphores, List<VkSemaphore>& presentImageSemaphores)
{

    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = NULL,
        .semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
        .initialValue = 0,
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCreateInfo
    };

    VkFenceCreateInfo fenceInfo =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    acquireImageSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    presentImageSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
    {
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &acquireImageSemaphores[x]));
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentImageSemaphores[x]));
        VULKAN_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[x]));
    }

    return VK_SUCCESS;
}