#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "VulkanSystem.h"
#include "TextureSystem.h"

class RenderSystem
{
public:
    static RenderSystem& Get();

private:
    RenderSystem() = default;
    ~RenderSystem() = default;
    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;

    void RecreateSwapchain(void* windowHandle, RenderPassGuid& renderPassGuid, LevelGuid& levelGuid, const float& deltaTime);
    void DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    Vector<VkFramebuffer> BuildFrameBuffer(const VulkanRenderPass& renderPass);
    VkDescriptorPool CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader);
    Vector<VkDescriptorSetLayout> CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader);
    Vector<VkDescriptorSet> AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
    void UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
    VkPipelineLayout CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
    VkPipeline CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
    void PipelineBindingData(RenderPipelineLoader& renderPipelineLoader);
    VkRenderPass BuildRenderPass(const RenderPassLoader& renderPassJsonLoader);

public:
   
    DLL_EXPORT void                          StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution);
    DLL_EXPORT VulkanRenderPass              RebuildSwapChain(VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    DLL_EXPORT void                          Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime);
    DLL_EXPORT void                          GenerateTexture(VkGuid& renderPassId);
    DLL_EXPORT const VulkanRenderPass        FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const Vector<VulkanPipeline>  FindRenderPipelineList(const RenderPassGuid& renderPassGuid);

    DLL_EXPORT void                          Destroy();
    DLL_EXPORT void                          DestroyRenderPass(VulkanRenderPass& renderPass);
    DLL_EXPORT void                          DestroyRenderPasses();
    DLL_EXPORT void                          DestroyRenderPipelines();
    DLL_EXPORT void                          DestroyPipeline(VulkanPipeline& vulkanPipelineDLL);
    DLL_EXPORT void                          DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                          DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                          DestroyBuffer(VkBuffer& buffer);

    Vector<VkDescriptorBufferInfo>    GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo>    GetMeshPropertiesBuffer(const  LevelGuid& levelGuid);
    Vector<VkDescriptorImageInfo>     GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid);
};
extern DLL_EXPORT RenderSystem& renderSystem;
inline RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}