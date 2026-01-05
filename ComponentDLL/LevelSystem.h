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

        RenderPassGuid brdfRenderPassId;
        RenderPassGuid directionalShadowRenderPassId;
        RenderPassGuid sdfShaderRenderPassId;
        RenderPassGuid skyBoxRenderPassId;
        RenderPassGuid irradianceMapRenderPassId;
        RenderPassGuid gBufferRenderPassId;
        RenderPassGuid geometryRenderPassId;
        RenderPassGuid verticalGaussianBlurRenderPassId;
        RenderPassGuid horizontalGaussianBlurRenderPassId;
        RenderPassGuid bloomRenderPassId;
        RenderPassGuid hdrRenderPassId;
        RenderPassGuid frameBufferId;
        RenderPassGuid shadowDebugRenderPassId;
       
        RenderPassGuid levelWireFrameRenderPass2DId;
        RenderPassGuid spriteWireFrameRenderPass2DId;

        DLL_EXPORT void                 Draw(VkCommandBuffer& commandBuffer, const float& deltaTime);
        DLL_EXPORT void                 RenderDirectionalShadowRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
        DLL_EXPORT void                 RenderSDFRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
        DLL_EXPORT void                 RenderSkyBox(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
        DLL_EXPORT void                 RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
        DLL_EXPORT void                 RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
        DLL_EXPORT void                 RenderGeometryRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
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
