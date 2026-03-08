#pragma once
#include "pch.h"
#include "VRAM.h"
#include "Camera.h"
#include "SpriteSystem.h"

#ifndef PLATFORM_ANDROID
#pragma comment(lib, "vulkan-1.lib")
#endif

struct GameObjectLoader
{
    String GameObjectPath;
    Vector<vec2> GameObjectPositionOverride;
};

struct LevelLoader
{
    VkGuid LevelID;
    Vector<String> LoadTextures;
    Vector<String> LoadMaterials;
    Vector<String> LoadSpriteVRAM;
    Vector<String> LoadTileSetVRAM;
    Vector<GameObjectLoader> GameObjectList;
    String LoadLevelLayout;
};

struct LevelLayer
{
    VkGuid				LevelId = VkGuid();
    uint				MeshId;
    VkGuid				MaterialId = VkGuid();
    VkGuid				TileSetId = VkGuid();
    int					LevelLayerIndex;
    ivec2				LevelBounds;
    uint*               TileIdMap;
    Tile*               TileMap;
    Vertex2DLayout*     VertexList;
    uint32*             IndexList;
    size_t				TileIdMapCount;
    size_t				TileMapCount;
    size_t				VertexListCount;
    size_t				IndexListCount;
};

class LevelSystem
{
public:
    static LevelSystem& Get();

private:
    LevelSystem() = default;
    ~LevelSystem() = default;
    LevelSystem(const LevelSystem&) = delete;
    LevelSystem& operator=(const LevelSystem&) = delete;
    LevelSystem(LevelSystem&&) = delete;
    LevelSystem& operator=(LevelSystem&&) = delete;

        bool WireframeModeFlag = false;

        LevelLayer  LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
        VkGuid      LoadTileSetVRAM(const char* tileSetPath);
        void        LoadLevelLayout(const char* levelLayoutPath);
        void        LoadLevelMesh(VkGuid& tileSetId);
        void        LoadSkyBox(const char* skyBoxMaterialPath);
        void        DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2DLayout* VertexList, uint32* IndexList);

    public:
        LevelLayout levelLayout;
        Vector<LevelLayer> LevelLayerList;
        Vector<Vector<uint>> LevelTileMapList;
        UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;
        SharedPtr<Camera> OrthographicCamera;
        SharedPtr<Camera> PerspectiveCamera;

        int UseHeightMap = 1;
        float HeightScale = 0.079f;
        vec3 ViewDirection = vec3(-0.037f, -0.062f, 1.0f);
        RenderPassGuid environmentToCubeMapRenderPassId;
        RenderPassGuid brdfRenderPassId;
        RenderPassGuid irradianceMapRenderPassId;
        RenderPassGuid prefilterMapRenderPassId;
        RenderPassGuid gBufferRenderPassId;
        RenderPassGuid verticalGaussianBlurRenderPassId;
        RenderPassGuid horizontalGaussianBlurRenderPassId;
        RenderPassGuid bloomRenderPassId;
        RenderPassGuid hdrRenderPassId;
        RenderPassGuid frameBufferId;
        RenderPassGuid shadowDebugRenderPassId;
       
        RenderPassGuid levelWireFrameRenderPass2DId;
        RenderPassGuid spriteWireFrameRenderPass2DId;
        void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount = 1)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? VK_ACCESS_SHADER_READ_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = layerCount
            };

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            printf("[Transition] Image %p %s ? %s (mips: %u, layers: %u)\n",
                (void*)image,
                oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ? "COLOR_ATTACHMENT" : "SHADER_READ",
                newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ? "SHADER_READ" : "COLOR_ATTACHMENT",
                mipLevels, layerCount);
        }
        void ResetAndDestroyOldTexture(uint32_t& mapId, Vector<Texture>& textureList)
        {
            if (mapId >= textureList.size()) return;

            Texture& tex = textureList[mapId];
            if (tex.textureImage == VK_NULL_HANDLE) return;

            // Reset layout if necessary
            VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.image = tex.textureImage;
            barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, tex.mipMapLevels, 0, 1 };

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);

            vulkanSystem.EndSingleUseCommand(cmd);
            printf("[ResetAndDestroy] Texture %p reset to COLOR_ATTACHMENT before destroy\n", (void*)tex.textureImage);

            // Now safe to destroy
            textureSystem.DestroyTexture(tex);
            mapId = UINT32_MAX;
        }
        void ResetAndDestroyOldCubemap(uint32_t& mapId, Vector<Texture>& cubemapList)
        {
            if (mapId >= cubemapList.size()) return;

            Texture& tex = cubemapList[mapId];
            if (tex.textureImage == VK_NULL_HANDLE) return;

            // Reset layout to COLOR_ATTACHMENT_OPTIMAL
            VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.image = tex.textureImage;
            barrier.subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = tex.mipMapLevels,
                .baseArrayLayer = 0,
                .layerCount = 6   // cubemap has 6 layers
            };

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);

            vulkanSystem.EndSingleUseCommand(cmd);
            printf("[ResetAndDestroyCubemap] Image %p reset to COLOR_ATTACHMENT before destroy\n", (void*)tex.textureImage);

            // Destroy
            textureSystem.DestroyTexture(tex);
            mapId = UINT32_MAX;
        }
        DLL_EXPORT void TransitionImageToShaderRead(VkCommandBuffer cmd, VkImage image, uint32_t mipLevels, uint32_t layerCount = 1);
        DLL_EXPORT void                 TransitionCubeMapToShaderRead(VkCommandBuffer cmd, VkImage cubeImage, uint32_t mipLevels);
        DLL_EXPORT void                 Draw(VkCommandBuffer& commandBuffer, const float& deltaTime);
        DLL_EXPORT void                 RenderEnvironmentToCubeMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
        DLL_EXPORT void                 RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
        DLL_EXPORT void                 RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
        DLL_EXPORT void                 RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection);
        DLL_EXPORT void                 RenderBloomPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 RenderShadowDebug(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 LoadLevel(const char* levelPath);
        DLL_EXPORT void                 Update(const float& deltaTime);
        DLL_EXPORT void                 DestroyLevel();
        DLL_EXPORT LevelLayout          GetLevelLayout();
        DLL_EXPORT Vector<LevelLayer>   GetLevelLayerList();
        DLL_EXPORT Vector<Vector<uint>> GetLevelTileMapList();
        DLL_EXPORT Vector<LevelTileSet> GetLevelTileSetList();
};
extern DLL_EXPORT LevelSystem& levelSystem;
inline LevelSystem& LevelSystem::Get()
{
    static LevelSystem instance;
    return instance;
}
