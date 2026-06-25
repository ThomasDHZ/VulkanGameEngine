#pragma once
#include <Platform.h>

class DLL_EXPORT VulkanInstance
{
private:
    uint32				                   m_apiVersion = VK_API_VERSION_1_1;
    VkInstance			                   m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR		                   m_surface = VK_NULL_HANDLE;

    void				                   SetUpVulkanInstance();
    void					               SetUpVulkanSurface();

    uint32                                 GetMaxApiVersion(VkPhysicalDevice physicalDevice);
    Vector<const char*>					   GetRequiredInstanceExtensions();

public:
    VulkanInstance();
    ~VulkanInstance();

     void                                   Initialize();
     Vector<const char*>                    GetValidationLayerProperties();

     [[nodiscard]] uint32            ApiVersion()     const;
     [[nodiscard]] VkInstance        InstanceHandle() const;
     [[nodiscard]] VkSurfaceKHR      Surface()        const;
};

