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
    uint				MeshId ;
    VkGuid				MaterialId = VkGuid();
    VkGuid				TileSetId = VkGuid();
    int					LevelLayerIndex;
    ivec2				LevelBounds;
    uint*               TileIdMap;
    Tile*               TileMap;
    Vertex2D*           VertexList;
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
        void        DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList);

    public:
        LevelLayout levelLayout;
        Vector<LevelLayer> LevelLayerList;
        Vector<Vector<uint>> LevelTileMapList;
        UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;
        SharedPtr<Camera> OrthographicCamera;

        RenderPassGuid levelRenderPass2DId;
        RenderPassGuid spriteRenderPass2DId;
        RenderPassGuid levelWireFrameRenderPass2DId;
        RenderPassGuid spriteWireFrameRenderPass2DId;
        RenderPassGuid frameBufferId;
        RenderPassGuid hdrRenderPassId;
        RenderPassGuid gaussianBlurRenderPassId;

        DLL_EXPORT void                 Draw(VkCommandBuffer& commandBuffer, const float& deltaTime);
        DLL_EXPORT void                 RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT VkCommandBuffer      RenderBloomPass(VkGuid& renderPassId);
        DLL_EXPORT void                 RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
        DLL_EXPORT void                 RenderLevel(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
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
