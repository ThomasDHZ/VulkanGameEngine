#include "LevelSystem.h"
#include "MaterialSystem.h"
#include "TextureSystem.h"
#include "GameObjectSystem.h"
#include "MeshSystem.h"
#include "VRAM.h"
#include "SpriteSystem.h"
#include "ShaderSystem.h"
#include "EngineConfigSystem.h"
#include <File.h>
#include "VulkanFileSystem.h"

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

    VkGuid dummyGuid = VkGuid();
    VkGuid tileSetId = VkGuid();

    shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str());

    nlohmann::json json = Json::ReadJson(levelPath);
    nlohmann::json shaderJson = Json::ReadJson("../RenderPass/LevelShader2DRenderPass.json");
    nlohmann::json shaderWiredJson = Json::ReadJson("../RenderPass/LevelShader2DWireFrameRenderPass.json");
    spriteRenderPass2DId = VkGuid(shaderJson["RenderPassId"].get<String>().c_str());
    levelWireFrameRenderPass2DId = VkGuid(shaderWiredJson["RenderPassId"].get<String>().c_str());
    shaderSystem.LoadShaderPipelineStructPrototypes(json["LoadRenderPasses"]);

    for (size_t x = 0; x < json["LoadTextures"].size(); x++)
    {
        textureSystem.LoadTexture(json["LoadTextures"][x]);
    }

    for (size_t x = 0; x < json["LoadMaterials"].size(); x++)
    {
        materialSystem.LoadMaterial(json["LoadMaterials"][x]);
    }

    for (size_t x = 0; x < json["LoadSpriteVRAM"].size(); x++)
    {
        spriteSystem.LoadSpriteVRAM(json["LoadSpriteVRAM"][x]);
    }

    for (size_t x = 0; x < json["LoadTileSetVRAM"].size(); x++)
    {
        tileSetId = LoadTileSetVRAM(json["LoadTileSetVRAM"][x]);
    }

    for (size_t x = 0; x < json["GameObjectList"].size(); x++)
    {
        String objectJson = json["GameObjectList"][x]["GameObjectPath"];
        vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
        gameObjectSystem.CreateGameObject(objectJson, positionOverride);
    }

    LoadLevelLayout(json["LoadLevelLayout"]);
    LoadLevelMesh(tileSetId);

    spriteSystem.AddSpriteBatchLayer(spriteRenderPass2DId);

    VkGuid LevelId = VkGuid(json["LevelID"].get<String>().c_str());
    spriteRenderPass2DId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "../RenderPass/LevelShader2DMultiSampledRenderPass.json", ivec2(renderSystem.renderer.SwapChainResolution.width, renderSystem.renderer.SwapChainResolution.height));
   // levelWireFrameRenderPass2DId = renderSystem.LoadRenderPass(levelLayout.LevelLayoutId, "../RenderPass/LevelShader2DWireFrameRenderPass.json", ivec2(renderSystem.renderer.SwapChainResolution.width, renderSystem.renderer.SwapChainResolution.height));
    frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "../RenderPass/FrameBufferRenderPass.json", ivec2(renderSystem.renderer.SwapChainResolution.width, renderSystem.renderer.SwapChainResolution.height));
}


void LevelSystem::Update(const float& deltaTime)
{
    OrthographicCamera->Update(*shaderSystem.GetGlobalShaderPushConstant("sceneData"));
    spriteSystem.Update(deltaTime);
    shaderSystem.UpdateGlobalShaderBuffer("sceneData");
}

void LevelSystem::Draw(Vector<VkCommandBuffer>& commandBufferList, const float& deltaTime)
{
    commandBufferList.emplace_back(renderSystem.RenderLevel(spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime, *shaderSystem.GetGlobalShaderPushConstant("sceneData")));
    commandBufferList.emplace_back(renderSystem.RenderFrameBuffer(frameBufferId));
}

void LevelSystem::DestroyDeadGameObjects()
{
    // Optional logic for cleaning up dead game objects
    // (commented out, implement as needed)
    /*
    if (gameObjectSystem.GameObjectList.empty()) return;

    Vector<SharedPtr<GameObject>> deadGameObjects;
    for (auto& gameObject : gameObjectSystem.GameObjectList)
    {
        if (!gameObject->GameObjectAlive)
            deadGameObjects.push_back(gameObject);
    }
    for (auto& gameObject : deadGameObjects)
    {
        // Remove sprite components if any
        // gameObject->Destroy();
    }
    */
}

VkGuid LevelSystem::LoadTileSetVRAM(const String& tileSetPath)
{
    if (tileSetPath.empty() || tileSetPath == "")
        return VkGuid();

    auto json = Json::ReadJson(tileSetPath);
    VkGuid tileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    if (LevelTileSetMap.find(tileSetId) != LevelTileSetMap.end())
        return tileSetId;

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoMapId);

    LevelTileSetMap[tileSetId] = VRAM_LoadTileSetVRAM(tileSetPath.c_str(), material, tileSetTexture);
    VRAM_LoadTileSets(tileSetPath.c_str(), LevelTileSetMap[tileSetId]);

    return tileSetId;
}

void LevelSystem::LoadLevelLayout(const String& levelLayoutPath)
{
    if (levelLayoutPath.empty() || levelLayoutPath == "")
        return;

    size_t levelLayerCount = 0;
    size_t levelLayerMapCount = 0;
    levelLayout = VRAM_LoadLevelInfo(levelLayoutPath.c_str());

    size_t levelLayerCountTemp = 0;
    size_t levelLayerMapCountTemp = 0;
    uint** levelLayerList = VRAM_LoadLevelLayout(levelLayoutPath.c_str(), levelLayerCountTemp, levelLayerMapCountTemp);

    Vector<uint*> levelMapPtrList(levelLayerList, levelLayerList + levelLayerCountTemp);
    for (size_t x = 0; x < levelLayerCountTemp; x++)
    {
        Vector<uint> mapLayer(levelMapPtrList[x], levelMapPtrList[x] + levelLayerMapCountTemp);
        LevelTileMapList.emplace_back(mapLayer);
        VRAM_DeleteLevelLayerMapPtr(levelMapPtrList[x]);
    }
    VRAM_DeleteLevelLayerPtr(levelLayerList);
}

void LevelSystem::LoadLevelMesh(VkGuid& tileSetId)
{
    for (size_t x = 0; x < LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = LevelTileSetMap[tileSetId];
        LevelLayerList.emplace_back(Level2D_LoadLevelInfo(levelLayout.LevelLayoutId, levelTileSet, LevelTileMapList[x].data(), LevelTileMapList[x].size(), levelLayout.LevelBounds, x));

        Vector<Vertex2D> vertexList(LevelLayerList[x].VertexList, LevelLayerList[x].VertexList + LevelLayerList[x].VertexListCount);
        Vector<uint> indexList(LevelLayerList[x].IndexList, LevelLayerList[x].IndexList + LevelLayerList[x].IndexListCount);
        meshSystem.CreateLevelLayerMesh(levelLayout.LevelLayoutId, vertexList, indexList);
    }
}

void LevelSystem::DestroyLevel()
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