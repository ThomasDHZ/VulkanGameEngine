#pragma once
#include "Sprite.h"
#include "SpriteBatchLayer.h"
#include "Level2D.h"
#include "RenderSystem.h"
#include "OrthographicCamera2D.h"
#include <VRAM.h>
#include <Level2D.h>

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

class MeshSystem;
class LevelSystem
{
private:
    VkGuid LoadTileSetVRAM(const String& tileSetPath);
    void LoadLevelLayout(const String& levelLayoutPath);
    void LoadLevelMesh(VkGuid& tileSetId);
    void DestroyDeadGameObjects();

public:
    RenderPassGuid levelRenderPass2DId;
    RenderPassGuid spriteRenderPass2DId;
    RenderPassGuid frameBufferId;
    SharedPtr<OrthographicCamera2D> OrthographicCamera;

    LevelLayout levelLayout;
    Vector<LevelLayer> LevelLayerList;
    Vector<Vector<uint>> LevelTileMapList;
    UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;

    LevelSystem();
    ~LevelSystem();

    void Update(const float& deltaTime);
    void Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime);
    void LoadLevel(const String& levelPath);
    void DestroyLevel();
};

extern LevelSystem levelSystem;