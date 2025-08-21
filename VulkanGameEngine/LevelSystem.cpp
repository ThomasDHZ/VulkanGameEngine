#include "LevelSystem.h"
#include "MaterialSystem.h"
#include "TextureSystem.h"
#include "GameObjectSystem.h"
#include "MeshSystem.h"
#include "VRAM.h"
#include "SpriteSystem.h"
#include "ShaderSystem.h"

LevelSystem levelSystem = LevelSystem();

LevelSystem::LevelSystem()
{
}

LevelSystem::~LevelSystem()
{
}

void LevelSystem::LoadLevel(const String& levelPath)
{
    OrthographicCamera = std::make_shared<OrthographicCamera2D>(OrthographicCamera2D(vec2((float)renderSystem.renderer.SwapChainResolution.width, (float)renderSystem.renderer.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));

    VkGuid tileSetId = VkGuid();
    nlohmann::json json = Json::ReadJson(levelPath);
    VkGuid LevelId = VkGuid(json["LevelID"].get<String>().c_str());
    for (int x = 0; x < json["LoadTextures"].size(); x++)
    {
        textureSystem.LoadTexture(json["LoadTextures"][x]);
    }
    for (int x = 0; x < json["LoadMaterials"].size(); x++)
    {
        materialSystem.LoadMaterial(json["LoadMaterials"][x]);
    }
    for (int x = 0; x < json["LoadSpriteVRAM"].size(); x++)
    {
        spriteSystem.LoadSpriteVRAM(json["LoadSpriteVRAM"][x]);
    }
    for (int x = 0; x < json["LoadTileSetVRAM"].size(); x++)
    {
        tileSetId = LoadTileSetVRAM(json["LoadTileSetVRAM"][x]);
    }
    for (int x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2   positionOverride = vec2(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }
    {
        LoadLevelLayout(json["LoadLevelLayout"]);
        LoadLevelMesh(tileSetId);
    }

    VkGuid dummyGuid = VkGuid();
    nlohmann::json json2 = Json::ReadJson("../RenderPass/LevelShader2DRenderPass.json");
    spriteRenderPass2DId = VkGuid(json2["RenderPassId"].get<String>().c_str());

    spriteSystem.AddSpriteBatchLayer(spriteRenderPass2DId);
    spriteRenderPass2DId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "../RenderPass/LevelShader2DRenderPass.json", ivec2(renderSystem.renderer.SwapChainResolution.width, renderSystem.renderer.SwapChainResolution.height));
    frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "../RenderPass/FrameBufferRenderPass.json", ivec2(renderSystem.renderer.SwapChainResolution.width, renderSystem.renderer.SwapChainResolution.height));
}

void LevelSystem::DestoryLevel()
{
    for (auto& tileMap : LevelTileSetMap)
    {
        VRAM_DeleteLevelVRAM(tileMap.second.LevelTileListPtr);
    }
    for (auto& levelLayer : LevelLayerList)
    {
        Level2D_DeleteLevel(levelLayer.TileIdMap, levelLayer.TileMap, levelLayer.VertexList, levelLayer.IndexList);
    }
}

void LevelSystem::Update(const float& deltaTime)
{
    OrthographicCamera->Update(*shaderSystem.GetGlobalShaderPushConstant("sceneData"));
    shaderSystem.GetPushConstantData(*shaderSystem.GetGlobalShaderPushConstant("sceneData"));
    spriteSystem.Update(deltaTime);
    shaderSystem.UpdateGlobalShaderBuffer("sceneData");
    for (auto& levelLayer : LevelLayerList)
    {
       // levelLayer.Update(deltaTime);
    }
}

void LevelSystem::Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime)
{
    commandBufferList.emplace_back(renderSystem.RenderLevel(spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime, *shaderSystem.GetGlobalShaderPushConstant("sceneData")));
    commandBufferList.emplace_back(renderSystem.RenderFrameBuffer(frameBufferId, spriteRenderPass2DId));
}

void LevelSystem::DestroyDeadGameObjects()
{
    //if (gameObjectSystem.GameObjectList.empty())
    //{
    //    return;
    //}

    //Vector<SharedPtr<GameObject>> deadGameObjectList;
    //for (auto it = GameObjectList.begin(); it != GameObjectList.end(); ++it) {
    //    if (!(*it)->GameObjectAlive) {
    //        deadGameObjectList.push_back(*it);
    //    }
    //}

    //if (!deadGameObjectList.empty())
    //{
    //    for (auto& gameObject : deadGameObjectList) {
    //        if (SharedPtr spriteComponent = gameObject->GetComponentByComponentType(kSpriteComponent)) {
    //            SharedPtr sprite = std::dynamic_pointer_cast<SpriteComponent>(spriteComponent);
    //            if (sprite) {
    //                SharedPtr spriteObject = sprite->GetSprite();
    //                SpriteLayerList[0]->RemoveSprite(spriteObject);
    //            }
    //        }
    //        gameObject->Destroy();
    //    }

    //    GameObjectList.erase(std::remove_if(GameObjectList.begin(), GameObjectList.end(),
    //        [&](const SharedPtr<GameObject>& gameObject) {
    //            return !gameObject->GameObjectAlive;
    //        }),
    //        GameObjectList.end());
    //}
}



VkGuid LevelSystem::LoadTileSetVRAM(const String& tileSetPath)
{
    if (tileSetPath.empty() ||
        tileSetPath == "")
    {
        return VkGuid();
    }

    nlohmann::json json = Json::ReadJson(tileSetPath);
    VkGuid tileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    auto it = LevelTileSetMap.find(tileSetId);
    if (it != LevelTileSetMap.end())
    {
        return tileSetId;
    }

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoMapId);

    LevelTileSetMap[tileSetId] = VRAM_LoadTileSetVRAM(tileSetPath.c_str(), material, tileSetTexture);
    VRAM_LoadTileSets(tileSetPath.c_str(), LevelTileSetMap[tileSetId]);

    return tileSetId;
}

void LevelSystem::LoadLevelLayout(const String& levelLayoutPath)
{
    if (levelLayoutPath.empty() ||
        levelLayoutPath == "")
    {
        return;
    }

    size_t levelLayerCount = 0;
    size_t levelLayerMapCount = 0;
    levelLayout = VRAM_LoadLevelInfo(levelLayoutPath.c_str());
    uint** levelLayerList = VRAM_LoadLevelLayout(levelLayoutPath.c_str(), levelLayerCount, levelLayerMapCount);
    Vector<uint*> levelMapPtrList = Vector<uint*>(levelLayerList, levelLayerList + levelLayerCount);
    for (size_t x = 0; x < levelLayerCount; x++)
    {
        Vector<uint> mapLayer = Vector<uint>(levelMapPtrList[x], levelMapPtrList[x] + levelLayerMapCount);
        LevelTileMapList.emplace_back(mapLayer);
        VRAM_DeleteLevelLayerMapPtr(levelMapPtrList[x]);
    }
    VRAM_DeleteLevelLayerPtr(levelLayerList);
}

void LevelSystem::LoadLevelMesh(VkGuid& tileSetId)
{
    for (int x = 0; x < LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = LevelTileSetMap[tileSetId];
        LevelLayerList.emplace_back(Level2D_LoadLevelInfo(levelLayout.LevelLayoutId, levelTileSet, LevelTileMapList[x].data(), LevelTileMapList[x].size(), levelLayout.LevelBounds, x));
       
        Vector<Vertex2D> vertexList = Vector<Vertex2D>(LevelLayerList[x].VertexList, LevelLayerList[x].VertexList + LevelLayerList[x].VertexListCount);
        Vector<uint> indexList = Vector<uint>(LevelLayerList[x].IndexList, LevelLayerList[x].IndexList + LevelLayerList[x].IndexListCount);
        meshSystem.CreateLevelLayerMesh(levelLayout.LevelLayoutId, vertexList, indexList);
    }
}