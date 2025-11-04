#pragma once
#include "pch.h"
#include <vulkan/vulkan_core.h>
#include <Vertex.h>
#include <MeshSystem.h>
#include "VRAM.h"
#include "Camera.h"
#include "SpriteSystem.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"
#include <RenderSystem.h>
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
    VkGuid				LevelId;
    uint				MeshId;
    VkGuid				MaterialId;
    VkGuid				TileSetId;
    int					LevelLayerIndex;
    ivec2				LevelBounds;
    uint* TileIdMap;
    Tile* TileMap;
    Vertex2D* VertexList;
    uint32* IndexList;
    size_t				TileIdMapCount;
    size_t				TileMapCount;
    size_t				VertexListCount;
    size_t				IndexListCount;
};

#ifdef __cplusplus
    extern "C" 
    {
        #endif
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderBloomPass(VkGuid& renderPassId);
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderFrameBuffer(VkGuid& renderPassId);
            DLL_EXPORT VkCommandBuffer LevelSystem_RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
            DLL_EXPORT void LevelSystem_LoadLevel(const char* levelPath);
            DLL_EXPORT void LevelSystem_Update(const float& deltaTime);
            DLL_EXPORT void LevelSystem_DestroyLevel();
        #ifdef __cplusplus
    }
#endif

    DLL_EXPORT LevelLayer Level2D_LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
    DLL_EXPORT void Level2D_DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList);
    DLL_EXPORT  VkGuid Level_LoadTileSetVRAM(const char* tileSetPath);
    DLL_EXPORT void Level_LoadLevelLayout(const GraphicsRenderer& renderer, const char* levelLayoutPath);
    DLL_EXPORT void Level_LoadLevelMesh(const GraphicsRenderer& renderer, VkGuid& tileSetId);
    DLL_EXPORT void Level_DestroyLevel();

class LevelSystem
{
private:
    bool WireframeModeFlag = false;

    VkGuid LoadTileSetVRAM(const String& tileSetPath) { return Level_LoadTileSetVRAM(tileSetPath.c_str()); }
    void LoadLevelLayout(const String& levelLayoutPath) { Level_LoadLevelLayout(renderer, levelLayoutPath.c_str()); }
    void LoadLevelMesh(VkGuid& tileSetId) { Level_LoadLevelMesh(renderer, tileSetId); }
    void DestroyDeadGameObjects() { Level_DestroyLevel(); }

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

    void Update(const float& deltaTime) 
    { 
        LevelSystem_Update(deltaTime); 
    }

    void Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime) 
    {
        commandBufferList.emplace_back(LevelSystem_RenderLevel(spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime));
        //commandBufferList.emplace_back(LevelSystem_RenderBloomPass(gaussianBlurRenderPassId));
        commandBufferList.emplace_back(LevelSystem_RenderFrameBuffer(frameBufferId));
    }

    void LoadLevel(const String& levelPath) 
    { 
        LevelSystem_LoadLevel(levelPath.c_str()); 
    }

    void DestroyLevel() 
    { 
        LevelSystem_DestroyLevel(); 
    }
};
DLL_EXPORT LevelSystem levelSystem;