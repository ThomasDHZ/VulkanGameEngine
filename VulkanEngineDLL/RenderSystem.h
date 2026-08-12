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

    UnorderedMap<VkGuid, VulkanRenderPass>                             RenderPassMap;
    UnorderedMap<VkGuid, VulkanPipelinePackage>                        RenderPipelinePackageMap;
    UnorderedMap<VkGuid, VulkanPipeline>                               RenderPipelineMap;

    DLL_EXPORT void                                                    RecreateSwapchain(void* windowHandle, const float& deltaTime);
    DLL_EXPORT void                                                    DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
    DLL_EXPORT void                                                    BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

public:
    bool                                                               WireFrameFlag = false;

    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(const String& jsonPath);
    DLL_EXPORT RenderPassGuid                                          LoadRenderPass(RenderPassLoader& renderPassLoader);
    DLL_EXPORT void                                                    Update(void* windowHandle, const float& deltaTime);
    DLL_EXPORT const VulkanRenderPass&                                 FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const VulkanPipelinePackage&                            FindPipelinePackage(const VkGuid& pipelinePackageGuid);
    DLL_EXPORT const VulkanPipeline&                                   FindRenderPipeline(const VkGuid& pipelineGuid);
    DLL_EXPORT bool                                                    FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineType pipelineType);
    DLL_EXPORT uint32                                                  SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);

    DLL_EXPORT void                                                    Draw(VkCommandBuffer& commandBuffer, Vector<RenderPassNode>& renderPassNodeList);

    DLL_EXPORT void                                                    DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer);
    DLL_EXPORT void                                                    DestroyBuffer(VkBuffer& buffer);
};
extern DLL_EXPORT RenderSystem& renderSystem;
inline RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}