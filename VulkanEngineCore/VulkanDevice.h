#pragma once
#include <Platform.h>

class DLL_EXPORT VulkanDevice
{
    friend class VulkanSwapchain;
private:
    VkPhysicalDevice                       m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                               m_logicalDevice = VK_NULL_HANDLE;
    VkQueue                                m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue                                m_presentQueue = VK_NULL_HANDLE;
    uint32                                 m_graphicsFamily = UINT32_MAX;
    uint32                                 m_presentFamily = UINT32_MAX;
    VkSampleCountFlagBits				   m_MaxSampleCount = VK_SAMPLE_COUNT_1_BIT;

    void                                   SetUpPhysicalDevice();
    void                                   SetUpLogicalDevice();
    void								   SetUpDeviceQueue();

    void                                   GetQueueFamilies(VkPhysicalDevice physicalDevice);
    VkSampleCountFlagBits				   GetMaxSampleCount(VkPhysicalDevice gpuDevice);
    Vector<const char*>					   GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
    bool								   GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList);

public:
    VulkanDevice();
    ~VulkanDevice();

    void                                   Initialize();
    Vector<VkSurfaceFormatKHR>			    GetSurfaceFormats(VkPhysicalDevice physicalDevice);//
    Vector<VkPresentModeKHR>			    GetSurfacePresentModes(VkPhysicalDevice physicalDevice);//

    Vector<VkPhysicalDevice>               GetPhysicalDeviceList(VkInstance instance);
    VkPhysicalDeviceProperties             GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);
    VkPhysicalDeviceFeatures               GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
    VkPhysicalDeviceFeatures2              GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice);
    Vector<VkSurfaceFormatKHR>             GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice);
    Vector<VkPresentModeKHR>               GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice);
    void                                   Shutdown();

    [[nodiscard]] VkPhysicalDevice         PhysicalDevice() const;
    [[nodiscard]] VkDevice                 LogicalDevice()  const;
    [[nodiscard]] VkQueue                  GraphicsQueue()  const;
    [[nodiscard]] VkQueue                  PresentQueue()   const;
    [[nodiscard]] uint32                   GraphicsFamily() const;
    [[nodiscard]] uint32                   PresentFamily()  const;
    [[nodiscard]] VkSampleCountFlagBits    MaxSampleCount() const;
};