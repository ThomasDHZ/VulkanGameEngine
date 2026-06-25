#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanSystem2.h"
#include <GLFW/glfw3.h>

VulkanSwapchain::VulkanSwapchain()
{
}

VulkanSwapchain::~VulkanSwapchain()
{
}

VulkanSwapchain::VulkanSwapchain(const VulkanSwapchain& other)
{
    m_SwapChainResolution = other.m_SwapChainResolution;
    m_ImageIndex = other.m_ImageIndex;
    m_CommandIndex = other.m_CommandIndex;
    m_SwapChainImageCount = other.m_SwapChainImageCount;
    m_renderResolution = other.m_renderResolution;
    m_Swapchain = other.m_Swapchain;
    m_InFlightFences = other.m_InFlightFences;
    m_SwapChainImages = other.m_SwapChainImages;
    m_SwapChainImageViews = other.m_SwapChainImageViews;
    m_AcquireImageSemaphores = other.m_AcquireImageSemaphores;
    m_PresentImageSemaphores = other.m_PresentImageSemaphores;
    m_RebuildSwapChainFlag = other.m_RebuildSwapChainFlag;
}

void VulkanSwapchain::Initialize(ivec2 renderResolution)
{
    m_ImageIndex = 0;
    m_CommandIndex = 0;
    m_renderResolution = renderResolution;
    StartUpSwapChain();
    StartUpSwapChainImages();
    StartUpSwapChainImageViews();
    StartUpSemaphores();
}

void VulkanSwapchain::StartUpSwapChain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities(vulkan.PhysicalDevice());
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = vulkan.Device().GetPhysicalDeviceFormats(vulkan.PhysicalDevice());
    Vector<VkPresentModeKHR> compatiblePresentModesList = vulkan.Device().GetPhysicalDevicePresentModes(vulkan.PhysicalDevice());
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);

    m_SwapChainImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0)
    {
        m_SwapChainImageCount = std::min(m_SwapChainImageCount, surfaceCapabilities.maxImageCount);
    }
    m_SwapChainImageCount = std::max(m_SwapChainImageCount, surfaceCapabilities.minImageCount);
    m_SwapChainResolution = StartUpSwapChainExtent();

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkan.Instance().Surface(),
        .minImageCount = m_SwapChainImageCount,
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = m_SwapChainResolution,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = m_Swapchain
    };

    if (vulkan.Device().GraphicsFamily() != vulkan.Device().PresentFamily())
    {
        Vector<uint32> queueFamilyIndices =
        {
            vulkan.Device().GraphicsFamily(),
            vulkan.Device().PresentFamily()
        };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_THROW_IF_FAIL(vkCreateSwapchainKHR(vulkan.Device().LogicalDevice(), &SwapChainCreateInfo, nullptr, &m_Swapchain));
}

void VulkanSwapchain::StartFrame()
{
    // Wait for the previous frame using this command index to finish
    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkan.LogicalDevice(), 1,
        &m_InFlightFences[m_CommandIndex], VK_TRUE, UINT64_MAX));
    VULKAN_THROW_IF_FAIL(vkResetFences(vulkan.LogicalDevice(), 1,
        &m_InFlightFences[m_CommandIndex]));

    // Acquire next swapchain image
    VkResult result = vkAcquireNextImageKHR(vulkan.LogicalDevice(), m_Swapchain, UINT64_MAX,
        m_AcquireImageSemaphores[m_CommandIndex], VK_NULL_HANDLE, &m_ImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        m_RebuildSwapChainFlag = true;
        return;
    }

    // Begin command buffer
    VkCommandBuffer cmd = vulkan.CommandBuffer().GetCurrentCommandBuffer();
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(cmd, &beginInfo));
}

void VulkanSwapchain::EndFrame(VkCommandBuffer& commandBufferSubmit)
{
    VkCommandBuffer cmd = vulkan.CommandBuffer().GetCurrentCommandBuffer();
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_AcquireImageSemaphores[m_CommandIndex];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_PresentImageSemaphores[m_CommandIndex];

    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkan.Device().GraphicsQueue(), 1, &submitInfo,
        m_InFlightFences[m_CommandIndex]));

    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_PresentImageSemaphores[m_CommandIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.pImageIndices = &m_ImageIndex;

    VkResult result = vkQueuePresentKHR(vulkan.Device().PresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        m_RebuildSwapChainFlag = true;
    }

    // Advance to next frame's command index
    m_CommandIndex = (m_CommandIndex + 1) % m_SwapChainImageCount;
}

void VulkanSwapchain::RebuildSwapChain(void* windowHandle)
{
    if (m_RebuildSwapChainFlag)
    {
        vkDeviceWaitIdle(vulkan.LogicalDevice());
        DestroySwapChainImageViews();
        DestroySwapChain();
        StartUpSwapChain();
        m_RebuildSwapChainFlag = false;
    }
}

void VulkanSwapchain::StartUpSwapChainImages()
{
    uint32 swapChainImageCount = UINT32_MAX;
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.Device().LogicalDevice(), m_Swapchain, &swapChainImageCount, nullptr));
    m_SwapChainImages.resize(swapChainImageCount);
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.Device().LogicalDevice(), m_Swapchain, &swapChainImageCount, m_SwapChainImages.data()));
}

VkSurfaceKHR VulkanSwapchain::StartUpVulkanSurface(void* windowHandle, VkInstance instance)
{
    if (!windowHandle || instance == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Invalid window handle (%p) or instance (%p)\n", windowHandle, (void*)instance);
        return VK_NULL_HANDLE;
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = (HWND)windowHandle;

    VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateWin32SurfaceKHR failed: %d (%s)\n",
            result,
            (result == VK_ERROR_EXTENSION_NOT_PRESENT) ? "VK_ERROR_EXTENSION_NOT_PRESENT" :
            (result == VK_ERROR_INITIALIZATION_FAILED) ? "VK_ERROR_INITIALIZATION_FAILED" :
            "unknown");
        return VK_NULL_HANDLE;
    }

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

    fprintf(stdout, "Surface created successfully: %p\n", (void*)surface);  // debug success
    return surface;
}

void VulkanSwapchain::StartUpSwapChainImageViews()
{
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = vulkan.Device().GetPhysicalDeviceFormats(vulkan.PhysicalDevice());
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);

    m_SwapChainImageViews.resize(m_SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < m_SwapChainImageCount; x++)
    {
        VkImageViewCreateInfo swapChainViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_SwapChainImages[x],
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
        VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkan.Device().LogicalDevice(), &swapChainViewInfo, nullptr, &m_SwapChainImageViews[x]));
    }
}

VkExtent2D VulkanSwapchain::StartUpSwapChainExtent()
{
    VkExtent2D extent;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities(vulkan.Device().PhysicalDevice());
    extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
    return extent;
}

void VulkanSwapchain::StartUpSemaphores()
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

    m_InFlightFences.resize(m_SwapChainImageCount, VK_NULL_HANDLE);
    m_AcquireImageSemaphores.resize(m_SwapChainImageCount, VK_NULL_HANDLE);
    m_PresentImageSemaphores.resize(m_SwapChainImageCount, VK_NULL_HANDLE);
    for (int x = 0; x < m_SwapChainImageCount; x++)
    {
        VULKAN_THROW_IF_FAIL(vkCreateFence(vulkan.Device().LogicalDevice(), &fenceInfo, NULL, &m_InFlightFences[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.Device().LogicalDevice(), &semaphoreCreateInfo, NULL, &m_AcquireImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.Device().LogicalDevice(), &semaphoreCreateInfo, NULL, &m_PresentImageSemaphores[x]));
    }
}

VkSurfaceFormatKHR VulkanSwapchain::FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR VulkanSwapchain::FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
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

Vector<VkSurfaceFormatKHR> VulkanSwapchain::GetSurfaceFormats(VkPhysicalDevice physicalDevice)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR>  surfaceFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, surfaceFormatList.data()));
    return surfaceFormatList;
}

Vector<VkPresentModeKHR> VulkanSwapchain::GetSurfacePresentModes(VkPhysicalDevice physicalDevice)
{
    uint32_t presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, NULL));
    Vector<VkPresentModeKHR> presentModeList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, presentModeList.data()));
    return presentModeList;
}

VkSurfaceCapabilitiesKHR VulkanSwapchain::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkan.Surface(), &surfaceCapabilities));
    return surfaceCapabilities;
}

void VulkanSwapchain::DestroySwapChainImageViews()
{
    for (auto& swapChainImageView : m_SwapChainImageViews)
    {
        if (swapChainImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(vulkan.LogicalDevice(), swapChainImageView, nullptr);
            swapChainImageView = VK_NULL_HANDLE;
        }
    }
    m_SwapChainImageViews.clear();
}

void VulkanSwapchain::DestroySwapChain()
{
    vkDestroySwapchainKHR(vulkan.LogicalDevice(), m_Swapchain, nullptr);
    m_Swapchain = VK_NULL_HANDLE;
}

void VulkanSwapchain::NextCommandIndex()
{
    m_CommandIndex = (m_CommandIndex + 1) % m_SwapChainImageCount;
}

void						VulkanSwapchain::TriggerSwapChainFlag()       { m_RebuildSwapChainFlag = true; }
uint32*				        VulkanSwapchain::ImageIndex()		     { return &m_ImageIndex; }
VkSwapchainKHR*              VulkanSwapchain::Swapchain()            { return &m_Swapchain; }
uint32				        VulkanSwapchain::CommandIndex()		     { return m_CommandIndex; }
uint32				        VulkanSwapchain::SwapChainImageCount()   { return m_SwapChainImageCount; }
VkExtent2D			        VulkanSwapchain::SwapChainResolution()   { return m_SwapChainResolution; }
ivec2					    VulkanSwapchain::RenderPassResolution()  { return m_renderResolution; }
 Vector<VkImage>	    VulkanSwapchain::SwapChainImages()       { return m_SwapChainImages; }
 Vector<VkImageView>	VulkanSwapchain::SwapChainImageViews()   { return m_SwapChainImageViews; }