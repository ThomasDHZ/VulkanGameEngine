#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "VulkanSystem.h"
#include "TextureSystem.h"
#include <optional>

struct VulkanBindVertexBuffer
{
    VkDeviceSize offsets = 0;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
};

struct MeshDrawMessage
{
    uint32		   MeshId = UINT32_MAX;
    uint32	       Drawlayer = UINT32_MAX;
    uint32         VertexBufferBinding = 0;
    uint32		   VertexCount = 0;
    uint32		   IndexCount = 0;
    uint32		   InstanceCount = 1;
    uint32         FirstVertex = 0;
    uint32	       FirstIndex = 0;
    uint32	       StartInstanceIndex = 0;
    VkDeviceSize   VertexOffset = 0;
    VkDeviceSize   InstanceOffset = 0;
    VkBuffer	   VertexBuffer = VK_NULL_HANDLE;
    VkBuffer	   IndexBuffer = VK_NULL_HANDLE;
    VkBuffer       InstanceBuffer = VK_NULL_HANDLE;
};

struct VulkanDrawMessage
{
    VkGuid                            RenderPassGuid;
    VkGuid                            PipelineGuid;
    std::optional<String>             PushConstant;
    Vector<PushConstantUpdateRule>    PushConstantUpdateRules;
    Vector<MeshDrawMessage>           DrawMeshList;
    Vector<VkGuid>                    RenderPassInputs;
    Vector<VkGuid>                    RenderPassOutputs;
    bool                              OffScreenRenderPass = false;

    std::function<void(VkCommandBuffer, VulkanDrawMessage&, uint32, ivec2 baseRenderPassSize, uint32 mipLevel)> PushConstantsCmd;
    std::function<void(VkCommandBuffer, VulkanDrawMessage)> PreDrawCmd;
    std::function<void(VkCommandBuffer, VulkanDrawMessage)> CustomDrawCmd;
    std::function<void(VkCommandBuffer, VulkanDrawMessage)> PostDrawCmd;
};

struct RenderPassNode
{
    VkGuid                                                        RenderPassGuid;
    uint32                                                        MipCount = 0;
    Vector<Vector<VulkanDrawMessage>>                             SubPassDrawMessage;

    std::function<void(VkCommandBuffer, RenderPassNode&)>         PreRenderPassCmd;
    std::function<void(VkCommandBuffer, RenderPassNode&)>         PostRenderPassCmd;
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

    Vector<VulkanRenderPass>                                           VulkanRenderPassList;
    UnorderedMap<VkGuid, uint32>                                       GuidToRenderPassNodeIndex;

    DLL_EXPORT void                                                    RecreateSwapchain(void* windowHandle, const float& deltaTime);
    DLL_EXPORT void                                                    DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    DLL_EXPORT VkPipelineLayout                                        CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
    DLL_EXPORT VkPipeline                                              CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
    DLL_EXPORT void                                                    BuildRenderPass(VulkanRenderPass& renderPass, const RenderPassLoader& renderPassJsonLoader);
    DLL_EXPORT void                                                    BuildPipelines(VulkanRenderPass& renderPass, const VulkanSubPassLoader& subPassLoader);
    DLL_EXPORT VulkanSubPass                                           BuildSubpasses(VkGuid& renderPassId, const VulkanSubPassLoader& subPassLoader);
    DLL_EXPORT Vector<VkAttachmentDescription>                         BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT Vector<Texture>                                         BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT void                                                    BuildFrameBuffer(VulkanRenderPass& renderPass);

    DLL_EXPORT void                                                    BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, ivec2 renderPassResolution, uint mipLevel = 0);
    DLL_EXPORT void                                                    BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindViewPort(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindViewPort(VkCommandBuffer& commandBuffer, ivec2 renderPassResolution, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    DLL_EXPORT void                                                    BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet = 0);
    DLL_EXPORT void                                                    NextSubpass(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                                                    EndRenderPass(VkCommandBuffer& commandBuffer);

public:

    Vector<RenderPassNode>                                             RenderPassNodeList;
    bool                                                               UsingMaterialBaker = false;
    UnorderedMap<RenderPassGuid, Vector<RenderPassAttachmentTexture>>  RenderPassAttachmentTextureInfoMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>               RenderPipelineMap;

    DLL_EXPORT void                                                    StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader);
    DLL_EXPORT void                                                    Update(void* windowHandle, const float& deltaTime);
    DLL_EXPORT const VulkanRenderPass&                                 FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT VulkanPipeline                                   FindRenderPipeline(const RenderPassGuid& renderPassGuid, const VkGuid& pipelineGuid);
    DLL_EXPORT const Vector<VulkanPipeline>                            FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT uint32                                                  SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);

    DLL_EXPORT void                                                    AddRenderNode(RenderPassNode renderPassNode);
    DLL_EXPORT void                                                    Draw(VkCommandBuffer& commandBuffer);

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
    Vector<VulkanRenderPass>&                                          RenderPassList();
};
extern DLL_EXPORT RenderSystem& renderSystem;
inline RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}