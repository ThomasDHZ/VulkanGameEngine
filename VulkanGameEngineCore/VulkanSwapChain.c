#include "VulkanSwapChain.h"
#include "VulkanWindow.h"
#include "VulkanRenderer.h"

static VkSurfaceFormatKHR SwapChain_FindSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint32* availableFormatsCount)
{
	for (uint32 x = 0; x < *availableFormatsCount; x++)
	{
		if (availableFormats[x].format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableFormats[x].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormats[x];
		}
	}
	fprintf(stderr, "Couldn't find a usable swap surface format.\n");
	return (VkSurfaceFormatKHR) { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
}

static VkPresentModeKHR SwapChain_FindSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint32* availablePresentModesCount)
{
	for (uint32 x = 0; x < *availablePresentModesCount; x++)
	{
		if (availablePresentModes[x] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentModes[x];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, uint32* graphicsFamily, uint32* presentFamily)
{
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

	VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
	if (queueFamilies)
	{
		for (uint32 x = 0; x <= queueFamilyCount; x++)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, renderer.Surface, &presentSupport);

			if (queueFamilies->queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				*presentFamily = x;
				*graphicsFamily = x;
				break;
			}
		}
		free(queueFamilies);
	}
	else
	{
		free(queueFamilies);
		Renderer_DestroyRenderer();
		SDL_Quit();
	}
}

VkResult Vulkan_SetUpSwapChain()
{
	VkSurfaceFormatKHR* compatibleSwapChainFormatList = NULL;
	VkPresentModeKHR* compatiblePresentModesList = NULL;
	uint32 surfaceFormatCount = 0;
	uint32 presentModeCount = 0;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VULKAN_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer.PhysicalDevice, renderer.Surface, &surfaceCapabilities));
	VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer.PhysicalDevice, renderer.Surface, &surfaceFormatCount, NULL));
	if (surfaceFormatCount != 0)
	{
		compatibleSwapChainFormatList = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
		VULKAN_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(renderer.PhysicalDevice, renderer.Surface, &surfaceFormatCount, compatibleSwapChainFormatList));
	}

	VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer.PhysicalDevice, renderer.Surface, &presentModeCount, NULL));
	if (presentModeCount != 0)
	{
		compatiblePresentModesList = malloc(sizeof(VkSurfaceFormatKHR) * presentModeCount);
		VULKAN_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(renderer.PhysicalDevice, renderer.Surface, &presentModeCount, compatiblePresentModesList));
	}

	SwapChain_GetQueueFamilies(renderer.PhysicalDevice, &renderer.SwapChain.GraphicsFamily, &renderer.SwapChain.PresentFamily);
	VkSurfaceFormatKHR SwapChainImageFormat = SwapChain_FindSwapSurfaceFormat(compatibleSwapChainFormatList, &surfaceFormatCount);
	VkPresentModeKHR SwapChainPresentMode = SwapChain_FindSwapPresentMode(compatiblePresentModesList, &presentModeCount);

	int width = 0;
	int height = 0;
	vulkanWindow->GetFrameBufferSize(vulkanWindow, &width, &height);

	VkExtent2D extent = { (uint32)width, (uint32)height };
	renderer.SwapChain.SwapChainResolution = extent;

	uint32 SwapChainImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 &&
		SwapChainImageCount > surfaceCapabilities.maxImageCount)
	{
		SwapChainImageCount = surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapChainCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = renderer.Surface,
		.minImageCount = SwapChainImageCount,
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

	if (renderer.SwapChain.GraphicsFamily != renderer.SwapChain.PresentFamily)
	{
		uint32 queueFamilyIndices[] = { renderer.SwapChain.GraphicsFamily, renderer.SwapChain.PresentFamily };

		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapChainCreateInfo.queueFamilyIndexCount = 2;
		SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VULKAN_RESULT(vkCreateSwapchainKHR(renderer.Device, &SwapChainCreateInfo, NULL, &renderer.SwapChain.Swapchain));


	renderer.SwapChain.SwapChainImageCount = INT32_MAX;
	VULKAN_RESULT(vkGetSwapchainImagesKHR(renderer.Device, renderer.SwapChain.Swapchain, &renderer.SwapChain.SwapChainImageCount, NULL));


	renderer.SwapChain.SwapChainImages = malloc(sizeof(VkImage) * renderer.SwapChain.SwapChainImageCount);
	VULKAN_RESULT(vkGetSwapchainImagesKHR(renderer.Device, renderer.SwapChain.Swapchain, &renderer.SwapChain.SwapChainImageCount, renderer.SwapChain.SwapChainImages));

	renderer.SwapChain.SwapChainImageViews = malloc(sizeof(VkImageView) * renderer.SwapChain.SwapChainImageCount);
	if (renderer.SwapChain.SwapChainImageViews != NULL)
	{
		for (uint32 x = 0; x < renderer.SwapChain.SwapChainImageCount; x++)
		{
			VkImageViewCreateInfo SwapChainViewInfo =
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = renderer.SwapChain.SwapChainImages[x],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = SwapChainImageFormat.format,
				.subresourceRange =
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			VULKAN_RESULT(vkCreateImageView(renderer.Device, &SwapChainViewInfo, NULL, &renderer.SwapChain.SwapChainImageViews[x]));
		}
		free(compatibleSwapChainFormatList);
		free(compatiblePresentModesList);
	}
	else
	{
		free(compatibleSwapChainFormatList);
		free(compatiblePresentModesList);
		Renderer_DestroyRenderer();
		SDL_Quit();
	}

	return VK_SUCCESS;
}

VkResult Vulkan_RebuildSwapChain()
{
	return Vulkan_SetUpSwapChain();
}

void Vulkan_DestroyImageView()
{
	for (uint32 x = 0; x < renderer.SwapChain.SwapChainImageCount; x++)
	{
		if (renderer.Surface != VK_NULL_HANDLE)
		{
			vkDestroyImageView(renderer.Device, renderer.SwapChain.SwapChainImageViews[x], NULL);
			renderer.SwapChain.SwapChainImageViews[x] = VK_NULL_HANDLE;
		}
	}
}

void Vulkan_DestroySwapChain()
{
	vkDestroySwapchainKHR(renderer.Device, renderer.SwapChain.Swapchain, NULL);
	renderer.SwapChain.Swapchain = VK_NULL_HANDLE;
}