using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using StbImageSharp;
using System.Numerics;
using System;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;
using System.Collections.Generic;

namespace VulkanGameEngineLevelEditor.Vulkan
{
    public unsafe class GameEngineImport
    {
        private const string DLLPath = "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\x64\\Debug\\VulkanEngineDLL.dll";

        ///Renderer
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern RendererStateCS Renderer_RendererSetUp_CS(void* windowHandle);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern TextureStruct Texture_LoadTexture_DLL(RendererStateCS rendererStateCS, string jsonString);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern TextureStruct Texture_CreateTexture_DLL(RendererStateCS rendererStateCS, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern TextureStruct Texture_CreateTexture_DLL(RendererStateCS rendererStateCS, string texturePath, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo, bool useMipMaps);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern TextureStruct Texture_CreateTexture_DLL(RendererStateCS rendererStateCS, Pixel clearColor, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo, bool useMipMaps);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void Texture_UpdateTextureSize_DLL(RendererStateCS rendererStateCS, TextureStruct texture, VkImageAspectFlagBits imageType, vec2 TextureResolution);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void Texture_UpdateTextureBufferIndex_DLL(TextureStruct texture, uint bufferIndex);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void Texture_GetTexturePropertiesBuffer_DLL(TextureStruct texture, List<VkDescriptorImageInfo> textureDescriptorList);

        [DllImport(DLLPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern VulkanRenderPassDLL* VulkanRenderPass_CreateVulkanRenderPassCS(
            ref RendererStateCS renderStateCS,
            [MarshalAs(UnmanagedType.LPStr)] string renderPassLoader,
            ref VkExtent2D renderPassResolution,
             int ConstBuffer,
                 ref        TextureStruct renderedTextureListPtr,
            ref ulong renderedTextureCount,
            ref TextureStruct depthTexture  );

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void VulkanRenderPass_DestroyRenderPass(RendererStateCS renderState, VulkanRenderPassDLL renderPass);


        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkInstance DLL_Renderer_CreateVulkanInstance();

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkDebugUtilsMessengerEXT DLL_Renderer_SetupDebugMessenger(VkInstance instance);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Renderer_GetDeviceQueue(VkDevice device, uint graphicsFamily, uint presentFamily, out VkQueue graphicsQueue, out VkQueue presentQueue);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, uint swapChainImageCount);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkPhysicalDevice DLL_Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint graphicsFamily, uint presentFamily);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkDevice DLL_Renderer_SetUpDevice(VkPhysicalDevice physicalDevice, uint graphicsFamily, uint presentFamily);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkPresentModeKHR* DLL_Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, out uint count);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Renderer_StartFrame(VkDevice device, VkSwapchainKHR swapChain, VkFence* fenceList, VkSemaphore* acquireImageSemaphoreList, uint* pImageIndex, uint* pCommandIndex, bool* pRebuildRendererFlag);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Renderer_EndFrame(VkSwapchainKHR swapChain, VkSemaphore* acquireImageSemaphoreList, VkSemaphore* presentImageSemaphoreList, VkFence* fenceList, VkQueue graphicsQueue, VkQueue presentQueue, uint commandIndex, uint imageIndex, VkCommandBuffer* pCommandBufferSubmitList, uint commandBufferCount, bool* rebuildRendererFlag);

        //SwapChain
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkSurfaceCapabilitiesKHR DLL_SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, out uint width, out uint height);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, out uint graphicsFamily, out uint presentFamily);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkSurfaceFormatKHR DLL_SwapChain_FindSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, uint count);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkPresentModeKHR DLL_SwapChain_FindSwapPresentMode(VkPresentModeKHR* availablePresentModes, uint count);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkSwapchainKHR DLL_SwapChain_SetUpSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint graphicsFamily, uint presentFamily, uint width, uint height, out uint swapChainImageCount);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkImage* DLL_SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint swapChainImageCount);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkImageView* DLL_SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, VkSurfaceFormatKHR swapChainImageFormat, uint swapChainImageCount);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkSurfaceFormatKHR* DLL_SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, out uint count);

        //Texture
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage* image, uint mipmapLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, out VkImage image, out VkDeviceMemory memory, VkImageCreateInfo ImageCreateInfo);
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_QuickTransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, uint mipmapLevels, ref VkImageLayout oldLayout, ref VkImageLayout newLayout);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_CommandBufferTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, uint mipmapLevels, VkImageLayout oldLayout, VkImageLayout newLayout);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_CopyBufferToTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkBuffer buffer, TextureUsageEnum textureType, int width, int height, int depth);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_GenerateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat* textureByteFormat, uint mipmapLevels, int width, int height, bool usingMipMaps);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_CreateTextureView(VkDevice device, out VkImageView view, VkImage image, VkFormat format, VkImageAspectFlagBits imageType, uint mipmapLevels);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Texture_CreateTextureSampler(VkDevice device, VkSamplerCreateInfo samplerCreateInfo, out VkSampler smapler);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Texture_UpdateTextureLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, out VkImageLayout oldImageLayout, VkImageLayout* newImageLayout, uint mipLevel);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Texture_UpdateCmdTextureLayout(VkCommandBuffer* commandBuffer, VkImage image, VkImageLayout oldImageLayout, out VkImageLayout newImageLayout, uint mipLevel);
        //  [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        //  public static extern void DLL_Texture_SaveTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, string filename, Texture texture, ExportTextureFormat textureFormat, uint channels)

        [DllImport(DLLPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void DLL_Texture_CreateImageTextureFromFile(VkDevice device,
                                                                                                                                                                   VkPhysicalDevice physicalDevice,
                                                                                                                                                                   VkCommandPool commandPool,
                                                                                                                                                                   VkQueue graphicsQueue,
                                                                                                                                                                   ref int width,
                                                                                                                                                                   ref int height,
                                                                                                                                                                   ref int depth,
                                                                                                                                                                   VkFormat textureByteFormat,
                                                                                                                                                                   uint mipmapLevels,
                                                                                                                                                                   ref VkImage textureImage,
                                                                                                                                                                   ref VkDeviceMemory textureMemory,
                                                                                                                                                                   ref VkImageLayout textureImageLayout,
                                                                                                                                                                   ref ColorComponents colorChannelUsed,
                                                                                                                                                                   TextureUsageEnum textureUsage,
                                                                                                                                                                   string filePath,
                                                                                                                                                                   bool useMipMap);
        [DllImport(DLLPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void DLL_Texture_CreateImageTextureFromClearColor(VkDevice device,
                                                                                                                                                                         VkPhysicalDevice physicalDevice,
                                                                                                                                                                         VkCommandPool commandPool,
                                                                                                                                                                         VkQueue graphicsQueue,
                                                                                                                                                                         ref int width,
                                                                                                                                                                         ref int height,
                                                                                                                                                                         ref int depth,
                                                                                                                                                                         VkFormat textureByteFormat,
                                                                                                                                                                         uint mipmapLevels,
                                                                                                                                                                         ref VkImage textureImage,
                                                                                                                                                                         ref VkDeviceMemory textureMemory,
                                                                                                                                                                         ref VkImageLayout textureImageLayout,
                                                                                                                                                                         ref ColorComponents colorChannelUsed,
                                                                                                                                                                         TextureUsageEnum textureUsage,
                                                                                                                                                                         Pixel filePath,
                                                                                                                                                                         bool useMipMap);
        //RenderPass
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkRenderPass DLL_RenderPass_BuildRenderPass(VkDevice device, RenderPassBuildInfoDLL renderPassBuildInfo);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkFramebuffer* DLL_RenderPass_BuildFrameBuffer(VkDevice device, VkRenderPass renderPass, RenderPassBuildInfoDLL renderPassBuildInfo, VkImageView* renderedColorTextureList, VkImageView* depthTextureView, VkImageView* swapChainImageViewList, uint swapChainCount, uint renderedTextureCount, ivec2 renderPassResolution);

        //Pipeline
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern unsafe VkDescriptorPool DLL_Pipeline_CreateDescriptorPool(VkDevice device, RenderPipelineDLL renderPipelineModel, GPUIncludes* includes);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkDescriptorSetLayout* DLL_Pipeline_CreateDescriptorSetLayout(VkDevice device, RenderPipelineDLL renderPipelineDLL, GPUIncludes includePtr);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkDescriptorSet* DLL_Pipeline_AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, RenderPipelineDLL renderPipelineDLL, VkDescriptorSetLayout* descriptorSetLayouts);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Pipeline_UpdateDescriptorSets(VkDevice device, RenderPipelineDLL renderPipelineDLL, GPUIncludes includePtr, VkDescriptorSet* descriptorSetList);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkPipelineLayout DLL_Pipeline_CreatePipelineLayout(VkDevice device, RenderPipelineDLL renderPipelineDLL, uint constBufferSize, VkDescriptorSetLayout* descriptorSetLayout);


        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkPipeline DLL_Pipeline_CreatePipeline(VkDevice device,
                                                                                                                                  VkRenderPass renderpass,
                                                                                                                                  VkPipelineLayout pipelineLayout,
                                                                                                                                  VkPipelineCache pipelineCache,
                                                                                                                                  RenderPipelineDLL model,
        ivec2 renderPassResolution);

        //Buffer
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_UpdateBufferMemory(VkDevice device, VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Buffer_CopyBufferMemory(VkDevice device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_AllocateMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlagBits properties);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void* DLL_Buffer_MapBufferMemory(VkDevice device, VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_UnmapBufferMemory(VkDevice device, VkDeviceMemory bufferMemory, bool* isMapped);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, out VkBuffer buffer, out VkDeviceMemory bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlagBits properties, VkBufferUsageFlagBits usage);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_CreateStagingBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, out VkBuffer stagingBuffer, out VkBuffer buffer, out VkDeviceMemory stagingBufferMemory, out VkDeviceMemory bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage, VkMemoryPropertyFlagBits properties);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_UpdateBufferSize(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize* oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlagBits bufferUsageFlags, VkMemoryPropertyFlagBits propertyFlags);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Buffer_UpdateBufferData(VkDevice device, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Buffer_UpdateStagingBufferData(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer stagingBuffer, VkBuffer buffer, out VkDeviceMemory stagingBufferMemory, out VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern VkResult DLL_Buffer_DestroyBuffer(VkDevice device, VkBuffer* buffer, VkBuffer* stagingBuffer, VkDeviceMemory* bufferMemory, VkDeviceMemory* stagingBufferMemory, void* bufferData, VkDeviceSize* bufferSize, VkBufferUsageFlagBits* bufferUsageFlags, VkMemoryPropertyFlagBits* propertyFlags);

        //Invoke Tools
        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern uint DLL_Tools_GetMemoryType(VkPhysicalDevice physicalDevice, uint typeFilter, VkMemoryPropertyFlagBits properties);

        [DllImport(DLLPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DLL_Tools_DeleteAllocatedPtr(void* ptr);
    }
}
