#pragma once

#include <Platform.h>
#include <VulkanSystem.h>
#include "SpriteSystem.h"
#include "ComponentSystem.h"
#include "MeshSystem.h"
#include <entt/entt.hpp>
#include "MaterialSystem.h"
#include "CameraSystem.h"

#ifndef PLATFORM_ANDROID
#pragma comment(lib, "vulkan-1.lib")
#endif

struct Tile
{
    uint                 TileId = 0;
    ivec2                TileUVCellOffset = ivec2();
    vec2                 TileUVOffset = vec2();
    int	                 TileLayer = 0;
    TileColliderTypeEnum TileCollider = kTileColliderNone;
    bool                 IsAnimatedTile = false;
};

struct GameObjectLoader
{
    String GameObjectPath;
    Vector<vec2> GameObjectPositionOverride;
};

struct LevelLoader
{
    VkGuid LevelID;
    Vector<String>           LoadTextures;
    Vector<String>           LoadMaterials;
    Vector<String>           LoadSpriteVRAM;
    Vector<String>           LoadTileSetVRAM;
    Vector<GameObjectLoader> GameObjectList;
    String                   LoadLevelLayout;
};

struct LevelLayer
{
    VkGuid				     LevelId = VkGuid();
    uint				     MeshId;
    VkGuid				     MaterialId = VkGuid();
    VkGuid				     TileSetId = VkGuid();
    int					     LevelLayerIndex;
    ivec2				     LevelBounds;
    Vector<uint>             TileIdMap;
    Vector<Tile>             TileMap;
    Vector<Vertex2DLayout>   VertexList;
    Vector<uint32>           IndexList;
};

struct LevelTileSet
{
    VkGuid			  TileSetId = VkGuid();
    VkGuid			  MaterialId = VkGuid();
    vec2			  TilePixelSize = vec2();
    ivec2			  TileSetBounds = ivec2();
    vec2			  TileScale = vec2(5.0f);
    vec2			  TileUVSize = vec2();
    Tile* LevelTileListPtr = nullptr;
    size_t            LevelTileCount = 0;
};

struct LevelLayout
{
    VkGuid					  LevelLayoutId;
    ivec2					  LevelBounds;
    ivec2					  TileSizeinPixels;
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

    Vector<VkGuid>                             RenderPassDrawList;
    bool                                       WireframeModeFlag = false;

    LevelLayer                                 LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
    VkGuid                                     LoadTileSetVRAM(const char* tileSetPath);
    void                                       LoadLevelLayout(const char* levelLayoutPath);
    LevelTileSet                               LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture);
    void                                       LoadTileSets(const char* tileSetPath, LevelTileSet& levelTileSet);
    LevelLayout                                LoadLevelInfo(const char* levelLayoutPath);
    void                                       LoadLevelMesh(VkGuid& tileSetId);
    void                                       LoadSkyBox();

public:
    VkGuid                                     PresentingAttachmentTextureId;
    LevelLayout                                levelLayout;
    Vector<LevelLayer>                         LevelLayerList;
    Vector<Vector<uint>>                       LevelTileMapList;
    UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;

    SharedPtr<Camera>                          PerspectiveCamera;

    uint                                       SelectedGameObject = UINT32_MAX;

    int                                        UseHeightMap = 1;
    float                                      HeightScale = 0.079f;
    vec3                                       ViewDirection = vec3(-0.037f, -0.062f, 1.0f);

    void                                       LoadLevel(const char* levelPath);
    void                                       LevelEditorRenderPass(const char* levelPath);
    void                                       Update(const float& deltaTime);

    Vector<RenderPassNode>                     CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime);
    LevelLayout                                GetLevelLayout();
    Vector<LevelLayer>                         GetLevelLayerList();
    Vector<Vector<uint>>                       GetLevelTileMapList();
    Vector<LevelTileSet>                       GetLevelTileSetList();
};
extern  LevelSystem& levelSystem;
inline LevelSystem& LevelSystem::Get()
{
    static LevelSystem instance;
    return instance;
}
