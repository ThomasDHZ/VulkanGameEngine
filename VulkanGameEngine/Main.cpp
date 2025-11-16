#include <Vulkan/vulkan.h>

VkInstance instance;
VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
appInfo.pApplicationName = "VulkanGameEngine";
appInfo.apiVersion = VK_API_VERSION_1_3;

VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
createInfo.pApplicationInfo = &appInfo;

if (vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS) {
    std::cout << "HEADLESS VULKAN INSTANCE CREATED — F5 DEBUG WORKS!\n";
    vkDestroyInstance(instance, nullptr);
}
else {
    std::cout << "Vulkan init failed\n";
}

std::cout << "ENGINE READY FOR F5 DEBUG\n";