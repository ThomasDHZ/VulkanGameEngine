#pragma once

#include <Platform.h>
#include "JsonStruct.h"
#include <VulkanSystem.h>
#include <VulkanPipeline.h>
#include "TextureSystem.h"
#include <optional>

struct VulkanBindVertexBuffer
{
    VkDeviceSize offsets = 0;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
};

struct VulkanDrawMessage
{
    VkGuid                            RenderPassGuid;
    VkGuid                            PipelinePackageGuid;
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
    Vector<Vector<VulkanDrawMessage>>                             SubPassDrawMessage;
    std::function<void(VkCommandBuffer, RenderPassNode&)>         PreRenderPassCmd;
    std::function<void(VkCommandBuffer, RenderPassNode&)>         PostRenderPassCmd;
    uint32                                                        MipCount = 0;
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

    UnorderedMap<VkGuid, VulkanRenderPass>                  RenderPassMap;
    UnorderedMap<VkGuid, VulkanPipelinePackage>             RenderPipelinePackageMap;
    UnorderedMap<VkGuid, VulkanPipeline>                    RenderPipelineMap;
    UnorderedMap<VkGuid, Vector<Texture>>                   RenderAttachmentMap;
    UnorderedMap<VkGuid, VulkanShader>                      RenderShaderMap;

#ifndef NDEBUG
    UnorderedMap<VkGuid, String>                            RenderAttachmentNameMap;
#endif

    VkGuid                                                  LoadShader(ShaderLoader& shaderLoader);
    VkGuid                                                  LoadPipeline(RenderPassLoader& renderPassLoader, VulkanPipelineLoader& pipelineLoader);
    void                                                    DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    void                                                    BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

public:
    bool                                                    WireFrameFlag = false;

    RenderPassGuid                                          LoadRenderPass(const String& jsonPath);
    RenderPassGuid                                          LoadRenderPass(RenderPassLoader& renderPassLoader);
    void                                                    Update(void* windowHandle, const float& deltaTime);
    uint32                                                  SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);
    void                                                    AddRenderedTexture(RenderPassGuid renderPassGuid, Vector<Texture>& renderedTextureList);

    void                                                    RecreateSwapchain(void* windowHandle, const float& deltaTime);
    void                                                    Draw(VkCommandBuffer& commandBuffer, Vector<RenderPassNode>& renderPassNodeList);

    const VulkanRenderPass&                                 FindRenderPass(const RenderPassGuid& renderPassGuid);
    const VulkanPipelinePackage&                            FindPipelinePackage(const VkGuid& pipelinePackageGuid);
    const VulkanPipeline&                                   FindRenderPipeline(const VkGuid& pipelineGuid);
    const VulkanShader&                                     FindVulkanShader(const VkGuid& shaderGuid);
    Texture&                                                FindRenderPassAttachment(const TextureGuid& textureGuid);
    Vector<Texture>&                                        FindRenderedTextureList(const RenderPassGuid& renderPassGuid);

    bool                                                    FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineTypeEnum pipelineType);
    bool                                                    RenderPassExists(const VkGuid& renderPassId);
    bool                                                    RenderPipelinePackageExists(const VkGuid& pipelinePackageId);
    bool                                                    RenderPipelineExists(const VkGuid& pipelineId);
    bool                                                    VulkanShaderExists(const VkGuid& vulkanShaderId);

    void                                                    Destroy();
};
extern  RenderSystem& renderSystem;
inline RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}