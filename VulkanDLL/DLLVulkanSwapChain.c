//#include "DLLVulkanSwapChain.h"
//
//VkSurfaceFormatKHR DLL_SwapChain_FindSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount) {
//    return SwapChain_FindSwapSurfaceFormat(availableFormats, availableFormatsCount);
//}
//
//VkPresentModeKHR DLL_SwapChain_FindSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount) {
//    return SwapChain_FindSwapPresentMode(availablePresentModes, availablePresentModesCount);
//}
//
//VkResult DLL_SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* graphicsFamily, uint32_t* presentFamily) {
//    return SwapChain_GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
//}
//
//VkResult DLL_SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* surfaceCapabilities) {
//    return SwapChain_GetSurfaceCapabilities(physicalDevice, surface, surfaceCapabilities);
//}
//
//VkResult DLL_SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR** compatibleSwapChainFormatList, uint32_t* surfaceFormatCount) {
//    return SwapChain_GetPhysicalDeviceFormats(physicalDevice, surface, compatibleSwapChainFormatList, surfaceFormatCount);
//}
//
//VkResult DLL_SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR** compatiblePresentModesList, uint32_t* presentModeCount) {
//    return SwapChain_GetPhysicalDevicePresentModes(physicalDevice, surface, compatiblePresentModesList, presentModeCount);
//}
//
//VkSwapchainKHR DLL_SwapChain_SetUpSwapChain(VkDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR surfaceCapabilities,
//    VkSurfaceFormatKHR swapChainImageFormat, VkPresentModeKHR swapChainPresentMode,
//    uint32_t graphicsFamily, uint32_t presentFamily, uint32_t width, uint32_t height,
//    uint32_t* swapChainImageCount) 
//{
//    return SwapChain_SetUpSwapChain(device, surface, surfaceCapabilities, swapChainImageFormat, swapChainPresentMode,
//        graphicsFamily, presentFamily, width, height, swapChainImageCount);
//}
//
//VkImage* DLL_SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32_t swapChainImageCount) {
//    return SwapChain_SetUpSwapChainImages(device, swapChain, swapChainImageCount);
//}
//
//VkImageView* DLL_SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImages, VkSurfaceFormatKHR* swapChainImageFormat, uint32_t swapChainImageCount) {
//    return SwapChain_SetUpSwapChainImageViews(device, swapChainImages, swapChainImageFormat, swapChainImageCount);
//}