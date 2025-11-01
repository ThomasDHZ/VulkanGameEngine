#include "CVulkanRenderer.h"
#include "CTypedef.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator)
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

VkResult Renderer_CreateSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight)
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

    for (int x = 0; x < maxFramesInFlight; x++)
    {
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &acquireImageSemaphores[x]));
        VULKAN_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &presentImageSemaphores[x]));
        VULKAN_RESULT(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[x]));
    }

    return VK_SUCCESS;
}

VkResult Renderer_CreateRenderPass(VkDevice device, RenderPassCreateInfoStruct* renderPassCreateInfo)
{
    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = renderPassCreateInfo->AttachmentCount,
        .pAttachments = renderPassCreateInfo->pAttachmentList,
        .subpassCount = renderPassCreateInfo->SubpassCount,
        .pSubpasses = renderPassCreateInfo->pSubpassDescriptionList,
        .dependencyCount = renderPassCreateInfo->DependencyCount,
        .pDependencies = renderPassCreateInfo->pSubpassDependencyList
    };
    return vkCreateRenderPass(device, &renderPassInfo, NULL, renderPassCreateInfo->pRenderPass);
}

VkResult Renderer_CreateDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool, VkDescriptorPoolCreateInfo* descriptorPoolCreateInfo)
{
    return vkCreateDescriptorPool(device, descriptorPoolCreateInfo, NULL, descriptorPool);
}

VkResult Renderer_CreateDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSetLayoutCreateInfo* descriptorSetLayoutCreateInfo)
{
    return vkCreateDescriptorSetLayout(device, descriptorSetLayoutCreateInfo, NULL, descriptorSetLayout);
}

VkResult Renderer_CreatePipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout, VkPipelineLayoutCreateInfo* pipelineLayoutCreateInfo)
{
    return vkCreatePipelineLayout(device, pipelineLayoutCreateInfo, NULL, pipelineLayout);
}

VkResult Renderer_AllocateDescriptorSets(VkDevice device, VkDescriptorSet* descriptorSet, VkDescriptorSetAllocateInfo* descriptorSetAllocateInfo)
{
    return vkAllocateDescriptorSets(device, descriptorSetAllocateInfo, descriptorSet);
}

VkResult Renderer_AllocateCommandBuffers(VkDevice device, VkCommandBuffer* commandBuffer, VkCommandBufferAllocateInfo* ImGuiCommandBuffers)
{
    return vkAllocateCommandBuffers(device, ImGuiCommandBuffers, commandBuffer);
}

VkResult Renderer_CreateGraphicsPipelines(VkDevice device, VkPipeline* graphicPipeline, VkGraphicsPipelineCreateInfo* createGraphicPipelines, uint32 createGraphicPipelinesCount)
{
    return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, createGraphicPipelinesCount, createGraphicPipelines, NULL, graphicPipeline);
}

VkResult Renderer_CreateCommandPool(VkDevice device, VkCommandPool* commandPool, VkCommandPoolCreateInfo* commandPoolInfo)
{
    return vkCreateCommandPool(device, commandPoolInfo, NULL, commandPool);
}

void Renderer_UpdateDescriptorSet(VkDevice device, VkWriteDescriptorSet* writeDescriptorSet, uint32 count)
{
    vkUpdateDescriptorSets(device, count, writeDescriptorSet, 0, NULL);
}

VkResult Renderer_StartFrame(VkDevice device, VkSwapchainKHR swapChain, VkFence* fenceList, VkSemaphore* acquireImageSemaphoreList, size_t* pImageIndex, size_t* pCommandIndex, bool* pRebuildRendererFlag)
{
    if (!pImageIndex ||
        !pCommandIndex ||
        !pRebuildRendererFlag)
    {
        fprintf(stderr, "Error: Null pointer passed to Renderer_StartFrame.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    *pCommandIndex = (*pCommandIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    VkResult waitResult = vkWaitForFences(device, 1, &fenceList[*pCommandIndex], VK_TRUE, UINT64_MAX);
    if (waitResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkWaitForFences failed with error code: %s\n", Renderer_GetError(waitResult));
        return waitResult;
    }

    VkResult resetResult = vkResetFences(device, 1, &fenceList[*pCommandIndex]);
    if (resetResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkResetFences failed with error code: %s\n", Renderer_GetError(resetResult));
        return resetResult;
    }

    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, acquireImageSemaphoreList[*pCommandIndex], VK_NULL_HANDLE, pImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        *pRebuildRendererFlag = true;
        return result;
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkAcquireNextImageKHR failed with error code: %s\n", Renderer_GetError(result));
        return result;
    }

    return result;
}

VkResult Renderer_EndFrame(VkSwapchainKHR swapChain, VkSemaphore* acquireImageSemaphoreList, VkSemaphore* presentImageSemaphoreList, VkFence* fenceList, VkQueue graphicsQueue, VkQueue presentQueue, size_t imageIndex, size_t commandIndex, VkCommandBuffer* pCommandBufferSubmitList, uint32_t commandBufferCount, bool* rebuildRendererFlag)
{
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &acquireImageSemaphoreList[commandIndex],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = commandBufferCount,
        .pCommandBuffers = pCommandBufferSubmitList,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &presentImageSemaphoreList[imageIndex]
    };

    VkResult submitResult = vkQueueSubmit(graphicsQueue, 1, &submitInfo, fenceList[commandIndex]);
    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
        submitResult == VK_SUBOPTIMAL_KHR)
    {
        *rebuildRendererFlag = true;
    }
    else if (submitResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkQueueSubmit failed with error code: %s\n", Renderer_GetError(submitResult));
        return submitResult;
    }

    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &presentImageSemaphoreList[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapChain,
        .pImageIndices = &imageIndex
    };

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        *rebuildRendererFlag = true;
    }
    else if (result != VK_SUCCESS) 
    {
        fprintf(stderr, "Error: vkQueuePresentKHR failed with error code: %s\n", Renderer_GetError(result));
    }

    return result;
}

VkResult Renderer_BeginCommandBuffer(VkCommandBuffer* pCommandBufferList, VkCommandBufferBeginInfo* commandBufferBeginInfo)
{
    return vkBeginCommandBuffer(*pCommandBufferList, commandBufferBeginInfo);
}

VkResult Renderer_EndCommandBuffer(VkCommandBuffer* pCommandBufferList)
{
    return vkEndCommandBuffer(*pCommandBufferList);
}

uint32_t Renderer_GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

VkCommandBuffer Renderer_BeginSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1
    };
    VULKAN_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

VkResult Renderer_EndSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    VULKAN_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_RESULT(vkQueueWaitIdle(graphicsQueue));
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    return VK_SUCCESS;
}

void Renderer_DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount)
{
    for (size_t x = 0; x < semaphoreCount; x++)
    {
        if (acquireImageSemaphores[x] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, acquireImageSemaphores[x], NULL);
            acquireImageSemaphores[x] = VK_NULL_HANDLE;
        }
        if (presentImageSemaphores[x] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, presentImageSemaphores[x], NULL);
            presentImageSemaphores[x] = VK_NULL_HANDLE;
        }
        if (fences[x] != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fences[x], NULL);
            fences[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyCommandPool(VkDevice device, VkCommandPool* commandPool)
{
    if (*commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, *commandPool, NULL);
        *commandPool = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDevice(VkDevice device)
{
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, NULL);
        device = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (*surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, *surface, NULL);
        *surface = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT)
{
    DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
}

void Renderer_DestroyInstance(VkInstance* instance)
{
    if (*instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(*instance, NULL);
        *instance = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
{
    if (*renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, *renderPass, NULL);
        *renderPass = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count)
{
    for (size_t x = 0; x < count; x++)
    {
        if (frameBufferList[x] != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device, frameBufferList[x], NULL);
            frameBufferList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
{
    if (*descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, *descriptorPool, NULL);
        *descriptorPool = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout)
{
    if (*descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, NULL);
        *descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count)
{
    if (*commandBufferList != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(device, *commandPool, count, &*commandBufferList);
        for (size_t x = 0; x < count; x++)
        {
            commandBufferList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroyBuffer(VkDevice device, VkBuffer* buffer)
{
    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, *buffer, NULL);
        *buffer = VK_NULL_HANDLE;
    }
}

void Renderer_FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory)
{
    if (*memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, *memory, NULL);
        *memory = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count)
{
    for (uint32 x = 0; x < count; x++)
    {
        if (surface != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, pSwapChainImageViewList[x], NULL);
            pSwapChainImageViewList[x] = VK_NULL_HANDLE;
        }
    }
}

void Renderer_DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain)
{
    vkDestroySwapchainKHR(device, *swapChain, NULL);
    *swapChain = VK_NULL_HANDLE;
}

void Renderer_DestroyImageView(VkDevice device, VkImageView* imageView)
{
    if (*imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, *imageView, NULL);
        *imageView = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyImage(VkDevice device, VkImage* image)
{
    if (*image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, *image, NULL);
        *image = VK_NULL_HANDLE;
    }
}

void Renderer_DestroySampler(VkDevice device, VkSampler* sampler)
{
    if (*sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, *sampler, NULL);
        *sampler = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipeline(VkDevice device, VkPipeline* pipeline)
{
    if (*pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, *pipeline, NULL);
        *pipeline = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout)
{
    if (*pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, *pipelineLayout, NULL);
        *pipelineLayout = VK_NULL_HANDLE;
    }
}

void Renderer_DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache)
{
    if (*pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, *pipelineCache, NULL);
        *pipelineCache = VK_NULL_HANDLE;
    }
}