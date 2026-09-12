#include "IblRenderSystem.h"
#include "RenderSystem.h"
#include "MeshSystem.h"
#include "from_json.h"

IblRenderSystem& iblRenderSystem = IblRenderSystem::Get();

void IblRenderSystem::StartUp(const String& texturePath)
{
    std::optional<MemoryPoolLoader> memoryPool = memoryPoolSystem.GetMemoryPoolInfo();
    _environmentMap = fileSystem.LoadJsonFile(texturePath.c_str())["TextureId"].get<VkGuid>();
    _brdfRenderPassId = renderSystem.LoadRenderPass("RenderPass/BRDFRenderPass.json", memoryPool);
    _environmentToCubeMapRenderPassId = renderSystem.LoadRenderPass("RenderPass/EnvironmentToCubeMapRenderPass.json", memoryPool);
    _irradianceMapRenderPassId = _renderPassDrawList.emplace_back(renderSystem.LoadRenderPass("RenderPass/IrradianceRenderPass.json", memoryPool));
    _prefilterMapRenderPassId = _renderPassDrawList.emplace_back(renderSystem.LoadRenderPass("RenderPass/PrefilterRenderPass.json", memoryPool));
    textureSystem.GenerateTexture(_brdfRenderPassId);
    SetEnvironmentMap(texturePath);
}

Vector<RenderPassNode> IblRenderSystem::CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
    Vector<RenderPassNode> renderPassNodeList;
    for (auto& renderPassGuid : _renderPassDrawList)
    {
        const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassGuid);

        uint32 maxMipLevelCount = 1;
        Vector<Vector<VulkanDrawMessage>> vulkanDrawMessageList;
        for (auto& renderPassList : renderPass.SubPassList())
        {
            Vector<VulkanDrawMessage> vulkanSubPassMessageList;
            for (auto& subPass : renderPassList)
            {
                for (auto& inputTexture : subPass.InputTextureList)
                {
                    const Texture& texture = renderSystem.FindRenderPassAttachment(inputTexture);
                    if (maxMipLevelCount < texture.texture.MipMapLevels()) maxMipLevelCount = texture.texture.MipMapLevels() - 1;
                }

                Vector<MeshDrawMessage> meshList;
                vulkanSubPassMessageList.emplace_back(VulkanDrawMessage
                    {
                        .RenderPassGuid = renderPassGuid,
                        .PipelinePackageGuid = subPass.PipelinePackageId,
                        .PushConstant = subPass.ShaderPushConstant,
                        .DrawMeshList = MeshTypeEnum::kMesh_StaticMesh && renderPass.RenderAsCubemap() ? meshSystem.DrawMesh("__SkyBoxMesh__") : meshSystem.DrawMesh(subPass.MeshType),
                        .RenderPassInputs = subPass.InputTextureList,
                        .RenderPassOutputs = subPass.OutputTextureList,
                        .OffScreenRenderPass = subPass.OffScreenFrameBuffer
                    });
            }
            vulkanDrawMessageList.emplace_back(vulkanSubPassMessageList);
        }
        renderPassNodeList.emplace_back(RenderPassNode
            {
               .RenderPassGuid = renderPassGuid,
               .SubPassDrawMessage = vulkanDrawMessageList,
               .MipCount = maxMipLevelCount
            });
    }
    return renderPassNodeList;
}

void IblRenderSystem::SetEnvironmentMap(const String& texturePath)
{
    vkDeviceWaitIdle(vulkan.LogicalDevice());
    if (_environmentMap != VkGuid())
    {
        textureSystem.DestroyTexture(_environmentMap);
        Texture texture = textureSystem.LoadTexture(texturePath);
        _environmentMap = texture.textureGuid;

        VulkanRenderPass& renderPass = renderSystem.FindRenderPass(_environmentToCubeMapRenderPassId);
        VkGuid pipelinePackage = renderPass.SubPassList().front().front().PipelinePackageId;

        renderPass.SubPassList().clear();
        renderPass.SubPassList().emplace_back(Vector<VulkanSubPass>
        {
            VulkanSubPass
            {
                .RenderPassGuid = _environmentToCubeMapRenderPassId,
                .PipelinePackageId = pipelinePackage,
                .MeshType = MeshTypeEnum::kMesh_StaticMesh,
                .ShaderPushConstant = std::nullopt,
                .InputTextureList = Vector<VkGuid>(),
                .OutputTextureList = Vector<VkGuid>(),
                .OffScreenFrameBuffer = true
            }
        });
    }
    textureSystem.GenerateTexture(_environmentToCubeMapRenderPassId);
}
