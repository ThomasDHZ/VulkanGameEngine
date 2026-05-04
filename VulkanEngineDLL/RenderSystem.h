#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "VulkanSystem.h"
#include "TextureSystem.h"
#include <optional>

enum class ResourceType { Texture, Buffer };

struct RenderPassResource 
{
    VkGuid TextureGuid;
    ResourceType Type;
    VkAccessFlags Access;           // e.g. VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    VkImageLayout Layout;           // final layout after this pass
    VkPipelineStageFlags Stage;     // stage that uses it
};



struct VulkanBindVertexBuffer
{
    VkDeviceSize offsets = 0;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
};

struct VulkanDrawMessage
{
    VkGuid                         RenderPassGuid;
    VkGuid                         PipelineGuid;
    std::optional<ShaderPushConstant> PushConstant;
    Vector<RenderPassResource>     RenderPassInputs;
    Vector<RenderPassResource>     RenderPassOutputs;
    Vector<VulkanBindVertexBuffer> VertexBufferList;
    VkBuffer                       IndexBuffer = VK_NULL_HANDLE;

    uint32                         MeshId = UINT32_MAX;
    uint32                         MipCount = 0;
    uint32                         FirstVertexBinding = 0;
    uint32                         VertexCount = 0;
    uint32                         IndexCount = 0;
    uint32                         InstanceCount = 1;
    uint32                         FirstIndex = 0;
    int32                          VertexOffset = 0;
    uint32                         FirstInstance = 0;
    
    std::function<void(VkCommandBuffer, VulkanDrawMessage)> PreDrawLayerCmd;
    std::function<void(VkCommandBuffer, VulkanDrawMessage&, ivec2 baseRenderPassSize, uint32 mipLevel)> PushConstantsCmd;
    std::function<void(VkCommandBuffer, VulkanDrawMessage)> PostDrawLayerCmd;

};

struct RenderPassNode
{
    uint32                                                        MipCount = 0;
    VkGuid                                                        RenderPassGuid;
    Vector<Vector<VulkanDrawMessage>>                             RenderPassDrawMessage;

    std::function<void(VkCommandBuffer, RenderPassNode&)>         PreRenderPassCmd;
    std::function<void(VkCommandBuffer, RenderPassNode&)>         PrepairSubpassCmd;
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

    Vector<RenderPassNode>                                             RenderPassNodess;
    bool                                                               UsingMaterialBaker = false;
    UnorderedMap<RenderPassGuid, Vector<RenderPassAttachmentTexture>>  RenderPassAttachmentTextureInfoMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>               RenderPipelineMap;

    DLL_EXPORT void                                                    StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader);
    DLL_EXPORT void                                                    Update(void* windowHandle, const float& deltaTime);
    DLL_EXPORT VulkanRenderPass                                        FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT VulkanPipeline                                          FindRenderPipeline(const RenderPassGuid& renderPassGuid, const VkGuid& pipelineGuid);
    DLL_EXPORT const Vector<VulkanPipeline>                            FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT uint32                                                  SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);

    DLL_EXPORT void                                                    AddRenderNode(RenderPassNode renderPassNode);

    DLL_EXPORT void                                                    BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, ivec2 renderPassResolution, uint mipLevel = 0);
    DLL_EXPORT void                                                    BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindViewPort(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindViewPort(VkCommandBuffer& commandBuffer, ivec2 renderPassResolution, uint mipLevel = 0);
    DLL_EXPORT void                                                    BindPushConstants(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, const ShaderPushConstant& pushConstant, VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    DLL_EXPORT void                                                    BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet = 0);
    DLL_EXPORT void                                                    DrawVertexMesh(VkCommandBuffer& commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
    DLL_EXPORT void                                                    DrawIndexedMesh(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage);
    DLL_EXPORT void                                                    DrawIndexedMesh(VkCommandBuffer& commandBuffer, Vector<VulkanDrawMessage>& vulkanDrawMessageList);
    DLL_EXPORT void                                                    NextSubpass(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                                                    EndRenderPass(VkCommandBuffer& commandBuffer);
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