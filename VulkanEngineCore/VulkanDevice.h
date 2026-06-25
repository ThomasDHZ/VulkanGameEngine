//#pragma once
//#include <Platform.h>
//
//class VulkanDevice
//{
//    friend class VulkanSwapchain;
//private:
//    VkPhysicalDevice                       m_physicalDevice = VK_NULL_HANDLE;
//    VkDevice                               m_logicalDevice = VK_NULL_HANDLE;
//    VkQueue                                m_graphicsQueue = VK_NULL_HANDLE;
//    VkQueue                                m_presentQueue = VK_NULL_HANDLE;
//    uint32                                 m_graphicsFamily = UINT32_MAX;
//    uint32                                 m_presentFamily = UINT32_MAX;
//    VkSampleCountFlagBits				   m_MaxSampleCount = VK_SAMPLE_COUNT_1_BIT;
//
//    void                                   SetUpPhysicalDevice();
//    void                                   SetUpLogicalDevice();
//    void								   SetUpDeviceQueue();
//
//    void                                   GetQueueFamilies(VkPhysicalDevice physicalDevice);
//    VkSampleCountFlagBits				   GetMaxSampleCount(VkPhysicalDevice gpuDevice);
//    Vector<const char*>					   GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
//    bool								   GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList);
//
//public:
//    VulkanDevice();
//    ~VulkanDevice();
//    void                                   Initialize();
//    DLL_EXPORT Vector<VkPhysicalDevice>    GetPhysicalDeviceList(VkInstance instance);
//    DLL_EXPORT VkPhysicalDeviceProperties  GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);
//    DLL_EXPORT VkPhysicalDeviceFeatures    GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
//    DLL_EXPORT VkPhysicalDeviceFeatures2   GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice);
//    DLL_EXPORT Vector<VkSurfaceFormatKHR>  GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice);
//    DLL_EXPORT Vector<VkPresentModeKHR>    GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice);
//    DLL_EXPORT void                        Shutdown();
//
//    [[nodiscard]] VkPhysicalDevice          PhysicalDevice() const { return m_physicalDevice; }
//    [[nodiscard]] VkDevice                  LogicalDevice()  const { return m_logicalDevice; }
//    [[nodiscard]] VkQueue                   GraphicsQueue()  const { return m_graphicsQueue; }
//    [[nodiscard]] VkQueue                   PresentQueue()   const { return m_presentQueue; }
//    [[nodiscard]] uint32                    GraphicsFamily() const { return m_graphicsFamily; }
//    [[nodiscard]] uint32                    PresentFamily()  const { return m_presentFamily; }
//    [[nodiscard]] VkSampleCountFlagBits     MaxSampleCount() const { return m_MaxSampleCount; }
//};