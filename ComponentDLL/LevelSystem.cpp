#include "pch.h"
#include "LevelSystem.h"
#include <RenderSystem.h>
#include "GameObjectSystem.h"

LevelSystem levelSystem = LevelSystem();

LevelLayer Level2D_LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex)
{
    Vector<Tile>     tileMap;
    Vector<uint32>   indexList;
    Vector<Vertex2D> vertexList;
    Vector<Tile>     tileSetList = Vector<Tile>(tileSet.LevelTileListPtr, tileSet.LevelTileListPtr + tileSet.LevelTileCount);
    Vector<uint>     tileIdMapList = Vector<uint>(tileIdMap, tileIdMap + tileIdMapCount);
    for (uint x = 0; x < levelBounds.x; x++)
    {
        for (uint y = 0; y < levelBounds.y; y++)
        {
            const uint& tileId = tileIdMapList[(y * levelBounds.x) + x];
            const Tile& tile = tileSetList[tileId];

            const float LeftSideUV = tile.TileUVOffset.x;
            const float RightSideUV = tile.TileUVOffset.x + tileSet.TileUVSize.x;
            const float TopSideUV = tile.TileUVOffset.y;
            const float BottomSideUV = tile.TileUVOffset.y + tileSet.TileUVSize.y;

            const uint VertexCount = vertexList.size();
            const vec2 TilePixelSize = tileSet.TilePixelSize * tileSet.TileScale;
            const Vertex2D BottomLeftVertex = { { x * TilePixelSize.x,                         y * TilePixelSize.y},                     {LeftSideUV, BottomSideUV} };
            const Vertex2D BottomRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,      y * TilePixelSize.y},                     {RightSideUV, BottomSideUV} };
            const Vertex2D TopRightVertex = { {(x * TilePixelSize.x) + TilePixelSize.x,     (y * TilePixelSize.y) + TilePixelSize.y},  {RightSideUV, TopSideUV} };
            const Vertex2D TopLeftVertex = { { x * TilePixelSize.x,                        (y * TilePixelSize.y) + TilePixelSize.y},  {LeftSideUV, TopSideUV} };

            vertexList.emplace_back(BottomLeftVertex);
            vertexList.emplace_back(BottomRightVertex);
            vertexList.emplace_back(TopRightVertex);
            vertexList.emplace_back(TopLeftVertex);

            indexList.emplace_back(VertexCount);
            indexList.emplace_back(VertexCount + 1);
            indexList.emplace_back(VertexCount + 2);
            indexList.emplace_back(VertexCount + 2);
            indexList.emplace_back(VertexCount + 3);
            indexList.emplace_back(VertexCount);

            tileMap.emplace_back(tile);
        }
    }

    LevelLayer levelLayout = LevelLayer
    {
        .LevelId = levelId,
        .MaterialId = tileSet.MaterialId,
        .TileSetId = tileSet.TileSetId,
        .LevelLayerIndex = levelLayerIndex,
        .LevelBounds = levelBounds,
        .TileIdMap = memorySystem.AddPtrBuffer<uint>(tileIdMapList.size(), __FILE__, __LINE__, __func__),
        .TileMap = memorySystem.AddPtrBuffer<Tile>(tileMap.size(), __FILE__, __LINE__, __func__),
        .VertexList = memorySystem.AddPtrBuffer<Vertex2D>(vertexList.size(), __FILE__, __LINE__, __func__),
        .IndexList = memorySystem.AddPtrBuffer<uint32>(indexList.size(), __FILE__, __LINE__, __func__),
        .TileIdMapCount = tileIdMapList.size(),
        .TileMapCount = tileMap.size(),
        .VertexListCount = vertexList.size(),
        .IndexListCount = indexList.size()
    };

    std::memcpy(levelLayout.TileMap, tileMap.data(), tileMap.size() * sizeof(Tile));
    std::memcpy(levelLayout.TileIdMap, tileIdMapList.data(), tileIdMapList.size() * sizeof(uint));
    std::memcpy(levelLayout.VertexList, vertexList.data(), vertexList.size() * sizeof(Vertex2D));
    std::memcpy(levelLayout.IndexList, indexList.data(), indexList.size() * sizeof(uint32));

    return levelLayout;
}

void Level2D_DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList)
{
    memorySystem.RemovePtrBuffer<uint>(TileIdMap);
    memorySystem.RemovePtrBuffer<Tile>(TileMap);
    memorySystem.RemovePtrBuffer<Vertex2D>(VertexList);
    memorySystem.RemovePtrBuffer<uint32>(IndexList);
}

VkGuid Level_LoadTileSetVRAM(const char* tileSetPath)
{
    if (!tileSetPath)
    {
        return VkGuid();
    }

    auto json = nlohmann::json::parse(File_Read(tileSetPath).Data);
    VkGuid tileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    if (levelSystem.LevelTileSetMap.find(tileSetId) != levelSystem.LevelTileSetMap.end())
        return tileSetId;

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoMapId);

    levelSystem.LevelTileSetMap[tileSetId] = VRAM_LoadTileSetVRAM(tileSetPath, material, tileSetTexture);
    VRAM_LoadTileSets(tileSetPath, levelSystem.LevelTileSetMap[tileSetId]);

    return tileSetId;
}

void Level_LoadLevelLayout(const GraphicsRenderer& renderer, const char* levelLayoutPath)
{
    if (!levelLayoutPath)
    {
        return;
    }

    size_t levelLayerCount = 0;
    size_t levelLayerMapCount = 0;
    levelSystem.levelLayout = VRAM_LoadLevelInfo(levelLayoutPath);

    size_t levelLayerCountTemp = 0;
    size_t levelLayerMapCountTemp = 0;
    uint** levelLayerList = VRAM_LoadLevelLayout(levelLayoutPath, levelLayerCountTemp, levelLayerMapCountTemp);

    Vector<uint*> levelMapPtrList(levelLayerList, levelLayerList + levelLayerCountTemp);
    for (size_t x = 0; x < levelLayerCountTemp; x++)
    {
        Vector<uint> mapLayer(levelMapPtrList[x], levelMapPtrList[x] + levelLayerMapCountTemp);
        levelSystem.LevelTileMapList.emplace_back(mapLayer);
        VRAM_DeleteLevelLayerMapPtr(levelMapPtrList[x]);
    }
    VRAM_DeleteLevelLayerPtr(levelLayerList);
}

void Level_LoadLevelMesh(const GraphicsRenderer& renderer, VkGuid& tileSetId)
{
    for (size_t x = 0; x < levelSystem.LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = levelSystem.LevelTileSetMap[tileSetId];
        levelSystem.LevelLayerList.emplace_back(Level2D_LoadLevelInfo(levelSystem.levelLayout.LevelLayoutId, levelTileSet, levelSystem.LevelTileMapList[x].data(), levelSystem.LevelTileMapList[x].size(), levelSystem.levelLayout.LevelBounds, x));

        Vector<Vertex2D> vertexList(levelSystem.LevelLayerList[x].VertexList, levelSystem.LevelLayerList[x].VertexList + levelSystem.LevelLayerList[x].VertexListCount);
        Vector<uint> indexList(levelSystem.LevelLayerList[x].IndexList, levelSystem.LevelLayerList[x].IndexList + levelSystem.LevelLayerList[x].IndexListCount);
        meshSystem.CreateSpriteLayerMesh(MeshTypeEnum::Mesh_LevelMesh, vertexList, indexList);
    }
}

void Level_DestroyLevel()
{
    spriteSystem.Destroy();
    for (auto& tileMap : levelSystem.LevelTileSetMap)
    {
        VRAM_DeleteLevelVRAM(tileMap.second.LevelTileListPtr);
    }

    for (auto& levelLayer : levelSystem.LevelLayerList)
    {
        Level2D_DeleteLevel(levelLayer.TileIdMap, levelLayer.TileMap, levelLayer.VertexList, levelLayer.IndexList);
    }
}

void LevelSystem_Update(const float& deltaTime)
{
    Camera_Update(*levelSystem.OrthographicCamera.get(), *shaderSystem.GetGlobalShaderPushConstant("sceneData"));
    spriteSystem.Update(deltaTime);
    shaderSystem.UpdateGlobalShaderBuffer("sceneData");
}

 void LevelSystem_Draw(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount, const float& deltaTime)
{

}

 LevelGuid LevelSystem_LoadLevel(const char* levelPath)
{
     levelSystem.OrthographicCamera = std::make_shared<Camera>(Camera_OrthographicCamera2D(vec2((float)renderer.SwapChainResolution.width, (float)renderer.SwapChainResolution.height), vec3(0.0f, 0.0f, 0.0f)));

     VkGuid dummyGuid = VkGuid();
     VkGuid tileSetId = VkGuid();

  //   shaderSystem.CompileShaders(configSystem.ShaderSourceDirectory.c_str(), configSystem.CompiledShaderOutputDirectory.c_str());

     nlohmann::json json = fileSystem.LoadJsonFile(levelPath);
     nlohmann::json shaderJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DRenderPass.json");
     nlohmann::json shaderWiredJson = fileSystem.LoadJsonFile("RenderPass/LevelShader2DWireFrameRenderPass.json");
     levelSystem.spriteRenderPass2DId = VkGuid(shaderJson["RenderPassId"].get<String>().c_str());
     levelSystem.levelWireFrameRenderPass2DId = VkGuid(shaderWiredJson["RenderPassId"].get<String>().c_str());
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
         tileSetId = Level_LoadTileSetVRAM(json["LoadTileSetVRAM"][x].get<String>().c_str());
     }

     for (size_t x = 0; x < json["GameObjectList"].size(); x++)
     {
         String objectJson = json["GameObjectList"][x]["GameObjectPath"];
         vec2 positionOverride(json["GameObjectList"][x]["GameObjectPositionOverride"][0], json["GameObjectList"][x]["GameObjectPositionOverride"][1]);
         gameObjectSystem.CreateGameObject(objectJson, positionOverride);
     }

     Level_LoadLevelLayout(renderer, json["LoadLevelLayout"].get<String>().c_str());
     Level_LoadLevelMesh(renderer, tileSetId);

     VkGuid levelId = VkGuid(json["LevelID"].get<String>().c_str());
     levelSystem.spriteRenderPass2DId = renderSystem.LoadRenderPass(levelSystem.levelLayout.LevelLayoutId, "RenderPass/LevelShader2DRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
     //    levelWireFrameRenderPass2DId = LoadRenderPass(levelLayout.LevelLayoutId, "RenderPass/LevelShader2DWireFrameRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
     levelSystem.gaussianBlurRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/GaussianBlurRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));
     levelSystem.frameBufferId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/FrameBufferRenderPass.json", ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height));

     return levelId;
}

 void LevelSystem_DestroyLevel()
{
}