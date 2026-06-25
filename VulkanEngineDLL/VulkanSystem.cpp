#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanSystem.h"
#include <cstdlib>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include "MemorySystem.h"
#include <Platform.h>
#include "EngineConfigSystem.h"
#include "BufferSystem.h"
#if defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

VulkanSystem& vulkanSystem = VulkanSystem::Get();
//LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;
//
//void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
//{
//    g_logVulkanMessageCallback = callback;
//}
//
//void VulkanSystem_LogVulkanMessage(const char* message, int severity)
//{
//    if (g_logVulkanMessageCallback)
//    {
//        g_logVulkanMessageCallback(message, severity);
//    }
//}

void VulkanSystem::VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution)
{
    vulkan.VulkanSetUp(windowResolution, renderResolution);
    RendererSetUp(vulkan.WindowHandle(), renderResolution);
}

void VulkanSystem::VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution)
{
    vulkan.VulkanSetUp(windowResolution, renderResolution);
    RendererSetUp(vulkan.WindowHandle(), renderResolution);
}

void VulkanSystem::RendererSetUp(const void* windowHandle, ivec2 renderResolution)
{
    vulkanSystem.RebuildRendererFlag = false;
    bufferSystem.vmaAllocator = SetUpVmaAllocation();
}

VmaAllocator VulkanSystem::SetUpVmaAllocation()
{
    VmaVulkanFunctions vulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };

    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = vulkan.PhysicalDevice(),
        .device = vulkan.LogicalDevice(),
        .preferredLargeHeapBlockSize = 64ull << 20,   // 64 MB
        .pVulkanFunctions = &vulkanFunctions,
        .instance = vulkan.InstanceHandle(),
        .vulkanApiVersion = vulkan.ApiVersion(),
    };

    VmaAllocator allocator = VK_NULL_HANDLE;
    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);

    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
    }

    return allocator;
}

void VulkanSystem::DestroyRenderer()
{
    //DestroySwapChainImageView(vulkan.LogicalDevice(), vulkanSystem.SwapChainImageViews);
    //DestroySwapChain(vulkan.LogicalDevice(), &vulkanSystem.Swapchain);
    //DestroyFences(vulkan.LogicalDevice(), vulkanSystem.AcquireImageSemaphores, vulkanSystem.PresentImageSemaphores, vulkanSystem.InFlightFences);
    //DestroyCommandPool(vulkan.LogicalDevice(), &vulkanSystem.CommandPool);
    //vmaDestroyAllocator(bufferSystem.vmaAllocator); 
    //DestroyDevice(vulkan.LogicalDevice());
    //DestroyDebugger(m_instance.InstanceHandle(), vulkanSystem.DebugMessenger);
    //DestroySurface(m_instance.InstanceHandle(), &m_instance.Surface());
    //DestroyInstance(m_instance.InstanceHandle());
}


Vector<const char*> VulkanSystem::GetValidationLayerProperties()
{
    uint32 layerCount = UINT32_MAX;
    Vector<const char*> validationLayers;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& layer : availableLayers)
            {
                if (strcmp(layer.layerName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            }
            std::cout << "Extension not supported: " << ext << '\n';
            //   __android_log_print(ANDROID_LOG_WARN, "Vulkan", "Validation layers not available - running without validation");
        };
    AddExtensionIfSupported("VK_LAYER_KHRONOS_validation");

    return extensions;
}

uint32_t VulkanSystem::GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t x = 0; x < memProperties.memoryTypeCount; x++)
    {
        if ((typeFilter & (1 << x)) &&
            (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
        {
            return x;
        }
    }

    return UINT32_MAX;
}

VkCommandBuffer VulkanSystem::BeginSingleUseCommand()
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkan.CommandBuffer().CommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkan.LogicalDevice(), &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

void VulkanSystem::EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkan.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_THROW_IF_FAIL(vkQueueWaitIdle(vulkan.GraphicsQueue()));

    vkFreeCommandBuffers(vulkan.LogicalDevice(), vulkan.CommandBuffer().CommandPool(), 1, &commandBuffer);
}

void VulkanSystem::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, debugUtilsMessengerEXT, pAllocator);
    }
    else
    {
        fprintf(stderr, "Failed to load vkDestroyDebugUtilsMessengerEXT function\n");
    }
}

void VulkanSystem::DestroyFences(VkDevice device, Vector<VkSemaphore>& acquireImageSemaphores, Vector<VkSemaphore>& presentImageSemaphores, Vector<VkFence>& fences)
{
    for (auto& acquireImageSemaphore : acquireImageSemaphores)
    {
        if (acquireImageSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, acquireImageSemaphore, NULL);
            acquireImageSemaphore = VK_NULL_HANDLE;
        }
    }
    acquireImageSemaphores.clear();

    for (auto& presentImageSemaphore : presentImageSemaphores)
    {
        if (presentImageSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, presentImageSemaphore, NULL);
            presentImageSemaphore = VK_NULL_HANDLE;
        }
    }
    presentImageSemaphores.clear();

    for (auto& fence : fences)
    {
        if (fence != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fence, NULL);
            fence = VK_NULL_HANDLE;
        }
    }
    fences.clear();
}

void VulkanSystem::DestroyCommandPool(VkDevice device, VkCommandPool* commandPool)
{
    if (*commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, *commandPool, NULL);
        *commandPool = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDevice(VkDevice device)
{
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, NULL);
        device = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (*surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, *surface, NULL);
        *surface = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT)
{
    DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
}

void VulkanSystem::DestroyInstance(VkInstance* instance)
{
    if (*instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(*instance, NULL);
        *instance = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
{
    if (*renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, *renderPass, NULL);
        *renderPass = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyFrameBuffers(VkDevice device, Vector<VkFramebuffer>& frameBufferList)
{
    for (auto& frameBuffer : frameBufferList)
    {
        if (frameBuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
            frameBuffer = VK_NULL_HANDLE;
        }
    }
    frameBufferList.clear();
}

void VulkanSystem::DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
{
    if (*descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, *descriptorPool, NULL);
        *descriptorPool = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout)
{
    if (*descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, NULL);
        *descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, Vector<VkCommandBuffer>& commandBufferList)
{
    vkFreeCommandBuffers(device, *commandPool, commandBufferList.size(), commandBufferList.data());
    for (size_t x = 0; x < commandBufferList.size(); x++)
    {
        commandBufferList[x] = VK_NULL_HANDLE;
    }
    commandBufferList.clear();
}

void VulkanSystem::DestroyBuffer(VkDevice device, VkBuffer* buffer)
{
    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, *buffer, NULL);
        *buffer = VK_NULL_HANDLE;
    }
}

void VulkanSystem::FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory)
{
    if (*memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, *memory, NULL);
        *memory = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySwapChainImageView(VkDevice device, Vector<VkImageView>& swapChainImageViewList)
{
    for (auto& swapChainImageView : swapChainImageViewList)
    {
        if (swapChainImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, swapChainImageView, nullptr);
            swapChainImageView = VK_NULL_HANDLE;
        }
    }
    swapChainImageViewList.clear();
}

void VulkanSystem::DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain)
{
    vkDestroySwapchainKHR(device, *swapChain, NULL);
    *swapChain = VK_NULL_HANDLE;
}

void VulkanSystem::DestroyImageView(VkDevice device, VkImageView* imageView)
{
    if (*imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, *imageView, NULL);
        *imageView = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyImage(VkDevice device, VkImage* image)
{
    if (*image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, *image, NULL);
        *image = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroySampler(VkDevice device, VkSampler* sampler)
{
    if (*sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, *sampler, NULL);
        *sampler = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipeline(VkDevice device, VkPipeline* pipeline)
{
    if (*pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, *pipeline, NULL);
        *pipeline = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout)
{
    if (*pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, *pipelineLayout, NULL);
        *pipelineLayout = VK_NULL_HANDLE;
    }
}

void VulkanSystem::DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache)
{
    if (*pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, *pipelineCache, NULL);
        *pipelineCache = VK_NULL_HANDLE;
    }
}