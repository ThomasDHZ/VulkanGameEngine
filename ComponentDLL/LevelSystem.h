#pragma once
#include "pch.h"
#include "VRAM.h"
#include "Camera.h"
#include "SpriteSystem.h"
#pragma comment(lib, "vulkan-1.lib")

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
    private:
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
        RenderPassGuid gaussianBlurRenderPassId;

        LevelSystem() {}
        ~LevelSystem() {}

        DLL_EXPORT void                 Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime);
        DLL_EXPORT VkCommandBuffer      RenderBloomPass(VkGuid& renderPassId);
        DLL_EXPORT VkCommandBuffer      RenderFrameBuffer(VkGuid& renderPassId);
        DLL_EXPORT VkCommandBuffer      RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
        DLL_EXPORT void                 LoadLevel(const char* levelPath);
        DLL_EXPORT void                 Update(const float& deltaTime);
        DLL_EXPORT void                 DestroyLevel();
        DLL_EXPORT LevelLayout          GetLevelLayout();
        DLL_EXPORT Vector<LevelLayer>   GetLevelLayerList();
        DLL_EXPORT Vector<Vector<uint>> GetLevelTileMapList();
        DLL_EXPORT Vector<LevelTileSet> GetLevelTileSetList();
};
DLL_EXPORT extern LevelSystem levelSystem;

#ifdef __cplusplus
    extern "C" 
    {
        #endif
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderBloomPass(VkGuid& renderPassId);
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderFrameBuffer(VkGuid& renderPassId);
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
            DLL_EXPORT void LevelSystem_LoadLevel(const char* levelPath);
            DLL_EXPORT void LevelSystem_Update(float deltaTime);
            DLL_EXPORT void LevelSystem_DestroyLevel();
            DLL_EXPORT LevelLayout LevelSystem_GetLevelLayout();
            DLL_EXPORT LevelLayer* LevelSystem_GetLevelLayerList(int& outCount);
            DLL_EXPORT uint** LevelSystem_GetLevelTileMapList(int& outCount);
            DLL_EXPORT LevelTileSet* LevelSystem_GetLevelTileSetList(int& outCount);
        #ifdef __cplusplus
    }
#endif
