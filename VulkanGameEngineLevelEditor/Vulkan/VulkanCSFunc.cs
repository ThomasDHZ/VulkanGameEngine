using System;
using System.Runtime.InteropServices;

namespace Vulkan
{
    public unsafe static class VkFunc
    {
        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkQueueSubmit(
            VkQueue queue,
            uint submitCount,
            VkSubmitInfo* pSubmits,
            VkFence fence);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkQueueWaitIdle(
            VkQueue queue);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkDeviceWaitIdle(
            VkDevice device);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkFreeMemory(
            VkDevice device,
            VkDeviceMemory memory,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkMapMemory(
            VkDevice device,
            VkDeviceMemory memory,
            ulong offset,
            ulong size,
            uint flags,
            void** ppData);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkUnmapMemory(
            VkDevice device,
            VkDeviceMemory memory);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkFlushMappedMemoryRanges(
            VkDevice device,
            uint memoryRangeCount,
            VkMappedMemoryRange* pMemoryRanges);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkInvalidateMappedMemoryRanges(
            VkDevice device,
            uint memoryRangeCount,
            VkMappedMemoryRange* pMemoryRanges);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetDeviceMemoryCommitment(
            VkDevice device,
            VkDeviceMemory memory,
            ulong* pCommittedMemoryInBytes);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkBindBufferMemory(
            VkDevice device,
            VkBuffer buffer,
            VkDeviceMemory memory,
            ulong memoryOffset);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkBindImageMemory(
            VkDevice device,
            VkImage image,
            VkDeviceMemory memory,
            ulong memoryOffset);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetBufferMemoryRequirements(
            VkDevice device,
            VkBuffer buffer,
            out VkMemoryRequirements pMemoryRequirements);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetImageMemoryRequirements(
            VkDevice device,
            VkImage image,
            out VkMemoryRequirements pMemoryRequirements);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetImageSparseMemoryRequirements(
            VkDevice device,
            VkImage image,
            uint* pSparseMemoryRequirementCount,
            VkSparseImageMemoryRequirements* pSparseMemoryRequirements);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetPhysicalDeviceSparseImageFormatProperties(
            VkPhysicalDevice physicalDevice,
            VkFormat format,
            VkImageType type,
            uint samples,
            VkImageUsageFlagBits usage,
            VkImageTiling tiling,
            uint* pPropertyCount,
            VkSparseImageFormatProperties* pProperties);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkQueueBindSparse(
            VkQueue queue,
            uint bindInfoCount,
            VkBindSparseInfo* pBindInfo,
            VkFence fence);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateFence(
            VkDevice device,
            in VkFenceCreateInfo pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkFence pFence);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyFence(
            VkDevice device,
            VkFence fence,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkResetFences(
            VkDevice device,
            uint fenceCount,
            VkFence* pFences);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkGetFenceStatus(
            VkDevice device,
            VkFence fence);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkWaitForFences(
            VkDevice device,
            uint fenceCount,
            VkFence* pFences,
            bool waitAll,
            ulong timeout);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateSemaphore(
            VkDevice device,
            in VkSemaphoreCreateInfo pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkSemaphore pSemaphore);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroySemaphore(
            VkDevice device,
            VkSemaphore semaphore,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateEvent(
            VkDevice device,
            VkEventCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkEvent* pEvent);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyEvent(
        VkDevice device,
            VkEvent Event,
        VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkGetEventStatus(
            VkDevice device,
            VkEvent Event);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkSetEvent(
            VkDevice device,
            VkEvent Event);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkResetEvent(
            VkDevice device,
            VkEvent Event);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateQueryPool(
            VkDevice device,
            VkQueryPoolCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkQueryPool* pQueryPool);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyQueryPool(
            VkDevice device,
            VkQueryPool queryPool,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkGetQueryPoolResults(
            VkDevice device,
            VkQueryPool queryPool,
            uint firstQuery,
            uint queryCount,
            IntPtr dataSize,
            void* pData,
            ulong stride,
            VkQueryResultFlagBits flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateBuffer(
            VkDevice device,
            VkBufferCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkBuffer pBuffer);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkAllocateMemory(
    VkDevice device,
     VkMemoryAllocateInfo* pAllocateInfo,
     VkAllocationCallbacks* pAllocator,
    out VkDeviceMemory pMemory);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyBuffer(
            VkDevice device,
            VkBuffer buffer,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateBufferView(
            VkDevice device,
            VkBufferViewCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkBufferView* pView);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyBufferView(
            VkDevice device,
            VkBufferView bufferView,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateImage(
            VkDevice device,
            VkImageCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkImage pImage);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyImage(
            VkDevice device,
            VkImage image,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetImageSubresourceLayout(
            VkDevice device,
            VkImage image,
            VkImageSubresource* pSubresource,
            VkSubresourceLayout* pLayout);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateImageView(
            VkDevice device,
            VkImageViewCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkImageView pView);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyImageView(
            VkDevice device,
            VkImageView imageView,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateShaderModule(
            VkDevice device,
            VkShaderModuleCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkShaderModule* pShaderModule);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyShaderModule(
            VkDevice device,
            VkShaderModule shaderModule,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreatePipelineCache(
            VkDevice device,
            VkPipelineCacheCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkPipelineCache* pPipelineCache);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyPipelineCache(
            VkDevice device,
            VkPipelineCache pipelineCache,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkGetPipelineCacheData(
            VkDevice device,
            VkPipelineCache pipelineCache,
            IntPtr pDataSize,
            void* pData);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkMergePipelineCaches(
            VkDevice device,
            VkPipelineCache dstCache,
            uint srcCacheCount,
            VkPipelineCache* pSrcCaches);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateGraphicsPipelines(
            VkDevice device,
            VkPipelineCache* pipelineCache,
            uint createInfoCount,
            VkGraphicsPipelineCreateInfo* pCreateInfos,
            VkAllocationCallbacks* pAllocator,
            out VkPipeline pPipelines);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateComputePipelines(
            VkDevice device,
            VkPipelineCache pipelineCache,
            uint createInfoCount,
            VkComputePipelineCreateInfo* pCreateInfos,
            VkAllocationCallbacks* pAllocator,
            VkPipeline* pPipelines);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyPipeline(
            VkDevice device,
            VkPipeline pipeline,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreatePipelineLayout(
            VkDevice device,
            VkPipelineLayoutCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkPipelineLayout pPipelineLayout);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyPipelineLayout(
            VkDevice device,
            VkPipelineLayout pipelineLayout,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateSampler(
            VkDevice device,
             VkSamplerCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkSampler pSampler);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroySampler(
            VkDevice device,
            VkSampler sampler,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateDescriptorSetLayout(
            VkDevice device,
            VkDescriptorSetLayoutCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkDescriptorSetLayout pSetLayout);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyDescriptorSetLayout(
            VkDevice device,
            VkDescriptorSetLayout descriptorSetLayout,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateDescriptorPool(
            VkDevice device,
            in VkDescriptorPoolCreateInfo pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            out VkDescriptorPool pDescriptorPool);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyDescriptorPool(
            VkDevice device,
            VkDescriptorPool descriptorPool,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkResetDescriptorPool(
            VkDevice device,
            VkDescriptorPool descriptorPool,
            VkDescriptorPoolCreateFlagBits flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkAllocateDescriptorSets(
            VkDevice device,
            VkDescriptorSetAllocateInfo* pAllocateInfo,
            out VkDescriptorSet pDescriptorSets);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkFreeDescriptorSets(
            VkDevice device,
            VkDescriptorPool descriptorPool,
            uint descriptorSetCount,
            VkDescriptorSet* pDescriptorSets);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkUpdateDescriptorSets(
            VkDevice device,
            uint descriptorWriteCount,
            VkWriteDescriptorSet* pDescriptorWrites,
            uint descriptorCopyCount,
            VkCopyDescriptorSet* pDescriptorCopies);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateFramebuffer(
            VkDevice device,
            VkFramebufferCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkFramebuffer* pFramebuffer);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateFramebuffer(
    VkDevice device,
    VkFramebufferCreateInfo* pCreateInfo,
    VkAllocationCallbacks* pAllocator,
    out VkFramebuffer pFramebuffer);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyFramebuffer(
            VkDevice device,
            VkFramebuffer framebuffer,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateRenderPass(
            VkDevice device,
            VkRenderPassCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkRenderPass* pRenderPass);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateRenderPass(
        VkDevice device,
        VkRenderPassCreateInfo* pCreateInfo,
        VkAllocationCallbacks* pAllocator,
        out VkRenderPass pRenderPass);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyRenderPass(
            VkDevice device,
            VkRenderPass renderPass,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetRenderAreaGranularity(
            VkDevice device,
            VkRenderPass renderPass,
            VkExtent2D* pGranularity);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateCommandPool(
            VkDevice device,
            VkCommandPoolCreateInfo* pCreateInfo,
            VkAllocationCallbacks* pAllocator,
            VkCommandPool* pCommandPool);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkDestroyCommandPool(
            VkDevice device,
            VkCommandPool commandPool,
            VkAllocationCallbacks* pAllocator);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkResetCommandPool(
            VkDevice device,
            VkCommandPool commandPool,
            VkCommandPoolResetFlagBits flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkAllocateCommandBuffers(
            VkDevice device,
            in VkCommandBufferAllocateInfo pAllocateInfo,
            out VkCommandBuffer pCommandBuffers);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern VkResult vkAcquireNextImageKHR(
              VkDevice device,
              VkSwapchainKHR swapchain,
              ulong timeout,
              VkSemaphore semaphore,
              VkFence fence,
              out uint imageIndex);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkCreateWin32SurfaceKHR(
    VkInstance instance,
     ref VkWin32SurfaceCreateInfoKHR pCreateInfo,
     VkAllocationCallbacks* pAllocator,
    out VkSurfaceKHR pSurface);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkFreeCommandBuffers(
            VkDevice device,
            VkCommandPool commandPool,
            uint commandBufferCount,
            VkCommandBuffer* pCommandBuffers);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkBeginCommandBuffer(
            VkCommandBuffer commandBuffer,
            VkCommandBufferBeginInfo* pBeginInfo);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkEndCommandBuffer(
            VkCommandBuffer commandBuffer);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern VkResult vkResetCommandBuffer(
            VkCommandBuffer commandBuffer,
            VkCommandBufferResetFlagBits flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBindPipeline(
            VkCommandBuffer commandBuffer,
            VkPipelineBindPoint pipelineBindPoint,
            VkPipeline pipeline);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetViewport(
            VkCommandBuffer commandBuffer,
            uint firstViewport,
            uint viewportCount,
            VkViewport* pViewports);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetScissor(
            VkCommandBuffer commandBuffer,
            uint firstScissor,
            uint scissorCount,
            VkRect2D* pScissors);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetLineWidth(
            VkCommandBuffer commandBuffer,
            float lineWidth);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetDepthBias(
            VkCommandBuffer commandBuffer,
            float depthBiasConstantFactor,
            float depthBiasClamp,
            float depthBiasSlopeFactor);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetBlendConstants(
            VkCommandBuffer commandBuffer,
            float* blendConstants);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetDepthBounds(
            VkCommandBuffer commandBuffer,
            float minDepthBounds,
            float maxDepthBounds);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetStencilCompareMask(
            VkCommandBuffer commandBuffer,
            VkStencilFaceFlagBits faceMask,
            uint compareMask);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetStencilWriteMask(
            VkCommandBuffer commandBuffer,
            VkStencilFaceFlagBits faceMask,
            uint writeMask);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetStencilReference(
            VkCommandBuffer commandBuffer,
            VkStencilFaceFlagBits faceMask,
            uint reference);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBindDescriptorSets(
            VkCommandBuffer commandBuffer,
            VkPipelineBindPoint pipelineBindPoint,
            VkPipelineLayout layout,
            uint firstSet,
            uint descriptorSetCount,
             VkDescriptorSet* pDescriptorSets,
            uint dynamicOffsetCount,
            uint* pDynamicOffsets);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBindIndexBuffer(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            ulong offset,
            VkIndexType indexType);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBindVertexBuffers(
            VkCommandBuffer commandBuffer,
            uint firstBinding,
            uint bindingCount,
            VkBuffer* pBuffers,
            ulong* pOffsets);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDraw(
            VkCommandBuffer commandBuffer,
            uint vertexCount,
            uint instanceCount,
            uint firstVertex,
            uint firstInstance);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDrawIndexed(
            VkCommandBuffer commandBuffer,
            uint indexCount,
            uint instanceCount,
            uint firstIndex,
            int vertexOffset,
            uint firstInstance);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDrawIndirect(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            ulong offset,
            uint drawCount,
            uint stride);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDrawIndexedIndirect(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            ulong offset,
            uint drawCount,
            uint stride);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDispatch(
            VkCommandBuffer commandBuffer,
            uint groupCountX,
            uint groupCountY,
            uint groupCountZ);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdDispatchIndirect(
            VkCommandBuffer commandBuffer,
            VkBuffer buffer,
            ulong offset);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdCopyBuffer(
            VkCommandBuffer commandBuffer,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            uint regionCount,
            VkBufferCopy* pRegions);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdCopyImage(
            VkCommandBuffer commandBuffer,
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkImage dstImage,
            VkImageLayout dstImageLayout,
            uint regionCount,
            VkImageCopy* pRegions);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBlitImage(
            VkCommandBuffer commandBuffer,
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkImage dstImage,
            VkImageLayout dstImageLayout,
            uint regionCount,
            VkImageBlit* pRegions,
            VkFilter filter);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdCopyBufferToImage(
            VkCommandBuffer commandBuffer,
            VkBuffer srcBuffer,
            VkImage dstImage,
            VkImageLayout dstImageLayout,
            uint regionCount,
            VkBufferImageCopy* pRegions);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkGetPhysicalDeviceFormatProperties(
            VkPhysicalDevice physicalDevice,
            VkFormat format,
            VkFormatProperties* pFormatProperties);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdCopyImageToBuffer(
            VkCommandBuffer commandBuffer,
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkBuffer dstBuffer,
            uint regionCount,
            VkBufferImageCopy* pRegions);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdUpdateBuffer(
            VkCommandBuffer commandBuffer,
            VkBuffer dstBuffer,
            ulong dstOffset,
            ulong dataSize,
            void* pData);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdFillBuffer(
            VkCommandBuffer commandBuffer,
            VkBuffer dstBuffer,
            ulong dstOffset,
            ulong size,
            uint data);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdClearColorImage(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkImageLayout imageLayout,
            VkClearColorValue* pColor,
            uint rangeCount,
            VkImageSubresourceRange* pRanges);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdClearDepthStencilImage(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkImageLayout imageLayout,
            VkClearDepthStencilValue* pDepthStencil,
            uint rangeCount,
            VkImageSubresourceRange* pRanges);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdClearAttachments(
            VkCommandBuffer commandBuffer,
            uint attachmentCount,
            VkClearAttachment* pAttachments,
            uint rectCount,
            VkClearRect* pRects);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdResolveImage(
            VkCommandBuffer commandBuffer,
            VkImage srcImage,
            VkImageLayout srcImageLayout,
            VkImage dstImage,
            VkImageLayout dstImageLayout,
            uint regionCount,
            VkImageResolve* pRegions);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdSetEvent(
            VkCommandBuffer commandBuffer,
            VkEvent Event,
        VkPipelineStageFlagBits stageMask);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdResetEvent(
            VkCommandBuffer commandBuffer,
            VkEvent eEvent,
        VkPipelineStageFlagBits stageMask);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdWaitEvents(
            VkCommandBuffer commandBuffer,
            uint eventCount,
            VkEvent* pEvents,
            VkPipelineStageFlagBits srcStageMask,
            VkPipelineStageFlagBits dstStageMask,
            uint memoryBarrierCount,
            VkMemoryBarrier* pMemoryBarriers,
            uint bufferMemoryBarrierCount,
            VkBufferMemoryBarrier* pBufferMemoryBarriers,
            uint imageMemoryBarrierCount,
            VkImageMemoryBarrier* pImageMemoryBarriers);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdPipelineBarrier(
            VkCommandBuffer commandBuffer,
            VkPipelineStageFlagBits srcStageMask,
            VkPipelineStageFlagBits dstStageMask,
            VkDependencyFlagBits dependencyFlags,
            uint memoryBarrierCount,
            VkMemoryBarrier* pMemoryBarriers,
            uint bufferMemoryBarrierCount,
            VkBufferMemoryBarrier* pBufferMemoryBarriers,
            uint imageMemoryBarrierCount,
            VkImageMemoryBarrier* pImageMemoryBarriers);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBeginQuery(
            VkCommandBuffer commandBuffer,
            VkQueryPool queryPool,
            uint query,
            VkQueryControlFlags flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdEndQuery(
            VkCommandBuffer commandBuffer,
            VkQueryPool queryPool,
            uint query);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdResetQueryPool(
            VkCommandBuffer commandBuffer,
            VkQueryPool queryPool,
            uint firstQuery,
            uint queryCount);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdWriteTimestamp(
            VkCommandBuffer commandBuffer,
            VkPipelineStageFlagBits pipelineStage,
            VkQueryPool queryPool,
            uint query);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdCopyQueryPoolResults(
            VkCommandBuffer commandBuffer,
            VkQueryPool queryPool,
            uint firstQuery,
            uint queryCount,
            VkBuffer dstBuffer,
            ulong dstOffset,
            ulong stride,
            VkQueryResultFlagBits flags);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdPushConstants(
            VkCommandBuffer commandBuffer,
            VkPipelineLayout layout,
            VkShaderStageFlagBits stageFlags,
            uint offset,
            uint size,
            void* pValues);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdNextSubpass(
            VkCommandBuffer commandBuffer,
            VkSubpassContents contents);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdEndRenderPass(
            VkCommandBuffer commandBuffer);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void vkCmdExecuteCommands(
            VkCommandBuffer commandBuffer,
            uint commandBufferCount,
            VkCommandBuffer* pCommandBuffers);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern VkResult vkCreateShaderModule(
        VkDevice device,
        ref VkShaderModuleCreateInfo pCreateInfo,
        IntPtr pAllocator,
        out VkShaderModule pShaderModule);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern void vkGetPhysicalDeviceMemoryProperties(
            VkPhysicalDevice physicalDevice,
            out VkPhysicalDeviceMemoryProperties pMemoryProperties);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
       VkPhysicalDevice physicalDevice,
       VkSurfaceKHR surface,
       out VkSurfaceCapabilitiesKHR pSurfaceCapabilities);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface,
            ref uint pSurfaceFormatCount,
            VkSurfaceFormatKHR* pSurfaceFormats);

        [DllImport("vulkan-1.dll", CallingConvention = CallingConvention.Winapi)]
        public static extern VkResult vkQueuePresentKHR(
       VkQueue queue,
       in VkPresentInfoKHR pPresentInfo);

    }
}
