//#pragma once
//#include "DLL.h"
//#include <VulkanSwapChain.h>
//
//DLL_EXPORT VkSurfaceFormatKHR DLL_SwapChain_FindSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint32_t availableFormatsCount);
//DLL_EXPORT VkPresentModeKHR DLL_SwapChain_FindSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint32_t availablePresentModesCount);
//DLL_EXPORT VkResult DLL_SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* graphicsFamily, uint32_t* presentFamily);
//DLL_EXPORT VkResult DLL_SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* surfaceCapabilities);
//DLL_EXPORT VkResult DLL_SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR** compatibleSwapChainFormatList, uint32_t* surfaceFormatCount);
//DLL_EXPORT VkResult DLL_SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR** compatiblePresentModesList, uint32_t* presentModeCount);
//DLL_EXPORT VkSwapchainKHR DLL_SwapChain_SetUpSwapChain(VkDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR surfaceCapabilities, VkSurfaceFormatKHR swapChainImageFormat, VkPresentModeKHR swapChainPresentMode, uint32_t graphicsFamily, uint32_t presentFamily, uint32_t width, uint32_t height, uint32_t* swapChainImageCount);
//DLL_EXPORT VkImage* DLL_SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32_t swapChainImageCount);
//DLL_EXPORT VkImageView* DLL_SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImages, VkSurfaceFormatKHR* swapChainImageFormat, uint32_t swapChainImageCount);
//
