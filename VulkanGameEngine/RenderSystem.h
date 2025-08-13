#pragma once
#include <CoreVulkanRenderer.h>
#include <VulkanRenderPass.h>
#include <ImGuiFunc.h>
#include "SceneDataBuffer.h"
#include <nlohmann/json.hpp>

typedef uint UM_SpriteID;
typedef uint UM_SpriteBatchID;
typedef uint UM_RenderPassID;
typedef uint UM_RenderPipelineID;
typedef uint UM_LevelID;
typedef VkGuid RenderPassGuid;
typedef VkGuid LevelGuid;

class RenderSystem
{
    friend class JsonRenderPass;
private:

    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    VkCommandBufferBeginInfo                                      CommandBufferBeginInfo;

    VkGuid CreateVulkanRenderPass(const String& jsonPath, ivec2& renderPassResolution);
    void RecreateSwapchain();

    const Vector<VkDescriptorBufferInfo> GetVertexPropertiesBuffer();
    const Vector<VkDescriptorBufferInfo> GetIndexPropertiesBuffer();
    const Vector<VkDescriptorBufferInfo> GetGameObjectTransformBuffer();
    const Vector<VkDescriptorBufferInfo> GetMeshPropertiesBuffer(VkGuid& levelLayerId);
    const Vector<VkDescriptorImageInfo>  GetTexturePropertiesBuffer(VkGuid& renderPassId, const Texture* renderedTexture);

    void DestroyRenderPass();
    void DestroyRenderPipeline();

public:

    ImGuiRenderer                                                 imGuiRenderer;

    RenderSystem();
    ~RenderSystem();

    void StartUp();
    void Update(const float& deltaTime);

    VkCommandBuffer RenderFrameBuffer(VkGuid& renderPassId);
    VkCommandBuffer RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime, SceneDataBuffer& sceneDataBuffer);
 
    VkGuid LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution);
    VkGuid LoadRenderPass(VkGuid& levelId, const String& jsonPath, Texture& inputTexture, ivec2 renderPassResolution);

    const VulkanRenderPass& FindRenderPass(const RenderPassGuid& guid);
    const Vector<VulkanPipeline>& FindRenderPipelineList(const RenderPassGuid& guid);

    void Destroy();
};
extern RenderSystem renderSystem;