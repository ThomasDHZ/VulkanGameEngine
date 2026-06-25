#include "VulkanDevice.h"
#include "VulkanSystem2.h"
#include "VulkanSwapchain.h"

VulkanDevice::VulkanDevice()
{
}

VulkanDevice::~VulkanDevice()
{
}

void VulkanDevice::Initialize()
{
    SetUpPhysicalDevice();
    SetUpLogicalDevice();
    SetUpDeviceQueue();
    m_MaxSampleCount = GetMaxSampleCount(m_physicalDevice);
}

void VulkanDevice::SetUpLogicalDevice()
{
    float queuePriority = 1.0f;
    Vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    if (m_graphicsFamily != UINT32_MAX)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_graphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }
    if (m_presentFamily != UINT32_MAX &&
        m_presentFamily != m_graphicsFamily)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_presentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }

    Vector<const char*> DeviceExtensionList = GetRequiredDeviceExtensions(m_physicalDevice);
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT,
        .pNext = &indexingFeatures,
        .colorWriteEnable = VK_TRUE
    };

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &colorWriteFeatures,
        .features =
        {
            .geometryShader = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_TRUE,
            .fragmentStoresAndAtomics = VK_TRUE,
            .shaderFloat64 = VK_TRUE,
            .shaderInt64 = VK_TRUE,
            .shaderInt16 = VK_TRUE,
        },
    };

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &physicalDeviceFeatures2,
        .storageBuffer8BitAccess = VK_TRUE,
        .uniformAndStorageBuffer8BitAccess = VK_TRUE,
        .descriptorIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .scalarBlockLayout = VK_TRUE,
        .separateDepthStencilLayouts = VK_TRUE,
        .timelineSemaphore = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
        .vulkanMemoryModel = VK_TRUE,
        .vulkanMemoryModelDeviceScope = VK_TRUE
    };

    VkPhysicalDeviceRobustness2FeaturesEXT physicalDeviceRobustness2Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan12Features,
        .nullDescriptor = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &physicalDeviceRobustness2Features,
        .shaderDemoteToHelperInvocation = VK_TRUE,
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
    Vector<const char*> validationLayers = vulkan.Instance().GetValidationLayerProperties();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    VULKAN_THROW_IF_FAIL(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice));
}

void VulkanDevice::SetUpPhysicalDevice()
{
    Vector<VkPhysicalDevice> physicalDeviceList = GetPhysicalDeviceList(vulkan.InstanceHandle());
    for (auto& physicalDevice : physicalDeviceList)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties = GetPhysicalDeviceProperties(physicalDevice);
        VkPhysicalDeviceFeatures physicalDeviceFeatures = GetPhysicalDeviceFeatures(physicalDevice);
        GetQueueFamilies(physicalDevice);
        Vector<VkSurfaceFormatKHR> surfaceFormatList = vulkan.Swapchain().GetSurfaceFormats(physicalDevice);
        Vector<VkPresentModeKHR> presentModeList = vulkan.Swapchain().GetSurfacePresentModes(physicalDevice);

        if (m_graphicsFamily != UINT32_MAX &&
            m_presentFamily != UINT32_MAX &&
            surfaceFormatList.size() > 0 &&
            presentModeList.size() > 0 &&
            physicalDeviceFeatures.samplerAnisotropy)
        {
            m_physicalDevice = physicalDevice;
            return;
        }
    }
}

void VulkanDevice::GetQueueFamilies(VkPhysicalDevice physicalDevice)
{
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    Vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, families.data());

    for (uint32 x = 0; x < queueFamilyCount; x++)
    {
        if (families[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsFamily = x;

            VkBool32 presentSupport = VK_FALSE;
            VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, vulkan.Surface(), &presentSupport));
            if (presentSupport) m_presentFamily = x;
            if (m_graphicsFamily != UINT32_MAX) break;
        }
    }
}

void VulkanDevice::SetUpDeviceQueue()
{
    if (m_graphicsFamily == UINT32_MAX || m_presentFamily == UINT32_MAX)
    {
        fprintf(stderr, "ERROR: Invalid queue family index!\n");
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    vkGetDeviceQueue(m_logicalDevice, m_graphicsFamily, 0, &m_graphicsQueue);
    if (m_graphicsQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: graphicsQueue is NULL! Family: %u (index 0 invalid?)\n", m_graphicsFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    vkGetDeviceQueue(m_logicalDevice, m_presentFamily, 0, &m_presentQueue);
    if (m_presentQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: presentQueue is NULL! Family: %u (index 0 invalid?)\n", m_presentFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }
    printf("SUCCESS: GraphicsQueue = %p (family %u), PresentQueue = %p (family %u)\n", (void*)m_graphicsQueue, m_graphicsFamily, (void*)m_presentQueue, m_presentFamily);
}

VkSampleCountFlagBits VulkanDevice::GetMaxSampleCount(VkPhysicalDevice gpuDevice)
{
    VkPhysicalDeviceLimits physicalDeviceProperties = GetPhysicalDeviceProperties(m_physicalDevice).limits;
    VkSampleCountFlags counts = physicalDeviceProperties.framebufferColorSampleCounts & physicalDeviceProperties.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

Vector<const char*> VulkanDevice::GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
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

VkPhysicalDeviceProperties VulkanDevice::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    return props;
}

VkPhysicalDeviceFeatures VulkanDevice::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    return features;
}

VkPhysicalDeviceFeatures2 VulkanDevice::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures2 features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    return features;
}

Vector<VkPhysicalDevice> VulkanDevice::GetPhysicalDeviceList(VkInstance instance)
{
    uint32 deviceCount = 0;
    Vector<VkPhysicalDevice> physicalDeviceList = Vector<VkPhysicalDevice>();
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    physicalDeviceList = Vector<VkPhysicalDevice>(deviceCount);
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data()));
    return physicalDeviceList;
}

Vector<VkSurfaceFormatKHR> VulkanDevice::GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, compatibleSwapChainFormatList.data()));
    return compatibleSwapChainFormatList;
}

Vector<VkPresentModeKHR> VulkanDevice::GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice)
{
    uint32 presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, nullptr));
    Vector<VkPresentModeKHR> compatiblePresentModesList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, compatiblePresentModesList.data()));
    return compatiblePresentModesList;
}

bool VulkanDevice::GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList)
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
            deviceExtensionList.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensionList.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            return true;
        }
        return false;
    }
    else
    {
        std::cout << "GPU/MotherBoard isn't ray tracing compatible." << std::endl;
        return false;
    }
    return false;
}

void VulkanDevice::Shutdown()
{
}

VkPhysicalDevice VulkanDevice::PhysicalDevice() const { return m_physicalDevice; }
VkDevice         VulkanDevice::LogicalDevice()  const { return m_logicalDevice; }
VkQueue          VulkanDevice::GraphicsQueue()  const { return m_graphicsQueue; }
VkQueue          VulkanDevice::PresentQueue()   const { return m_presentQueue; }
uint32           VulkanDevice::GraphicsFamily() const { return m_graphicsFamily; }
uint32           VulkanDevice::PresentFamily()  const { return m_presentFamily; }
VkSampleCountFlagBits VulkanDevice::MaxSampleCount() const { return m_MaxSampleCount; }