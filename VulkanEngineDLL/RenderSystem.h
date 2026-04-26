#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "VulkanSystem.h"
#include "TextureSystem.h"

enum class ResourceType { Texture, Buffer };

struct RenderPassResource 
{
    VkGuid RenderPassGuid;
    ResourceType Type;
    VkAccessFlags Access;           // e.g. VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    VkImageLayout Layout;           // final layout after this pass
    VkPipelineStageFlags Stage;     // stage that uses it
};

struct RenderPassNode
{
    VkGuid RenderPassGuid;
    Vector<RenderPassResource> RenderPassInputs;
    Vector<RenderPassResource> RenderPassOutputs;
    std::function<void(VkCommandBuffer commandBuffer, const VulkanRenderPass& renderPass)> Command;
};

class RenderSystem
{
public:
    static RenderSystem& Get();
    friend class MaterialBakerSystem;

private:
    RenderSystem() = default;
    ~RenderSystem() = default;
    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

    Vector<VulkanRenderPass>                                           RenderPassNodes;
    UnorderedMap<VkGuid, uint32>                                       GuidToRenderPassNodeIndex;

    DLL_EXPORT void                                                    RecreateSwapchain(void* windowHandle, const float& deltaTime);
    DLL_EXPORT void                                                    DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    DLL_EXPORT VkPipelineLayout                                        CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
    DLL_EXPORT VkPipeline                                              CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
    DLL_EXPORT void                                                    BuildRenderPass(VulkanRenderPass& renderPass, const RenderPassLoader& renderPassJsonLoader);
    DLL_EXPORT Vector<VkAttachmentDescription>                         BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT Vector<Texture>                                         BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT void                                                    BuildFrameBuffer(VulkanRenderPass& renderPass);
    DLL_EXPORT void                                                    AddRenderPass(const VulkanRenderPass& vulkanRenderPass);

public:

    bool                                                               UsingMaterialBaker = false;
    UnorderedMap<RenderPassGuid, Vector<RenderPassAttachmentTexture>>  RenderPassAttachmentTextureInfoMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>               RenderPipelineMap;

    DLL_EXPORT void                                                    StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader);
    DLL_EXPORT void                                                    Update(void* windowHandle, const float& deltaTime);
    DLL_EXPORT VulkanRenderPass                                        FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const Vector<VulkanPipeline>                            FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT uint32                                                  SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);

    DLL_EXPORT void                                                    Destroy();
    DLL_EXPORT void                                                    DestroyRenderPass(VulkanRenderPass& renderPass);
    DLL_EXPORT void                                                    DestroyRenderPasses();
    DLL_EXPORT void                                                    DestroyRenderPipelines();
    DLL_EXPORT void                                                    DestroyPipeline(VulkanPipeline& vulkanPipelineDLL);
    DLL_EXPORT void                                                    DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                                                    DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer);
    DLL_EXPORT void                                                    DestroyBuffer(VkBuffer& buffer);

    Vector<VkDescriptorImageInfo>                                      GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid);
    Vector<VkDescriptorImageInfo>                                      GetTexture3DPropertiesBuffer(const RenderPassGuid& renderPassGuid);
    Vector<VkDescriptorImageInfo>                                      GetCubeMapTextureBuffer();
    Vector<VulkanRenderPass>&                                    RenderPassList();
};
extern DLL_EXPORT RenderSystem& renderSystem;
inline RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}