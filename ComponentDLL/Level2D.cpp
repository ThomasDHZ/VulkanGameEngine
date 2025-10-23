#include "pch.h"
#include "Level2D.h"
#include "MemorySystem.h"

LevelArchive levelArchive = LevelArchive();

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

    if (levelArchive.LevelTileSetMap.find(tileSetId) != levelArchive.LevelTileSetMap.end())
        return tileSetId;

    const Material& material = materialSystem.FindMaterial(materialId);
    const Texture& tileSetTexture = textureSystem.FindTexture(material.AlbedoMapId);

    levelArchive.LevelTileSetMap[tileSetId] = VRAM_LoadTileSetVRAM(tileSetPath, material, tileSetTexture);
    VRAM_LoadTileSets(tileSetPath, levelArchive.LevelTileSetMap[tileSetId]);

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
    levelArchive.levelLayout = VRAM_LoadLevelInfo(levelLayoutPath);

    size_t levelLayerCountTemp = 0;
    size_t levelLayerMapCountTemp = 0;
    uint** levelLayerList = VRAM_LoadLevelLayout(levelLayoutPath, levelLayerCountTemp, levelLayerMapCountTemp);

    Vector<uint*> levelMapPtrList(levelLayerList, levelLayerList + levelLayerCountTemp);
    for (size_t x = 0; x < levelLayerCountTemp; x++)
    {
        Vector<uint> mapLayer(levelMapPtrList[x], levelMapPtrList[x] + levelLayerMapCountTemp);
        levelArchive.LevelTileMapList.emplace_back(mapLayer);
        VRAM_DeleteLevelLayerMapPtr(levelMapPtrList[x]);
    }
    VRAM_DeleteLevelLayerPtr(levelLayerList);
}

void Level_LoadLevelMesh(const GraphicsRenderer& renderer, VkGuid& tileSetId)
{
    for (size_t x = 0; x < levelArchive.LevelTileMapList.size(); x++)
    {
        const LevelTileSet& levelTileSet = levelArchive.LevelTileSetMap[tileSetId];
        levelArchive.LevelLayerList.emplace_back(Level2D_LoadLevelInfo(levelArchive.levelLayout.LevelLayoutId, levelTileSet, levelArchive.LevelTileMapList[x].data(), levelArchive.LevelTileMapList[x].size(), levelArchive.levelLayout.LevelBounds, x));

        Vector<Vertex2D> vertexList(levelArchive.LevelLayerList[x].VertexList, levelArchive.LevelLayerList[x].VertexList + levelArchive.LevelLayerList[x].VertexListCount);
        Vector<uint> indexList(levelArchive.LevelLayerList[x].IndexList, levelArchive.LevelLayerList[x].IndexList + levelArchive.LevelLayerList[x].IndexListCount);
        meshSystem.CreateSpriteLayerMesh(renderer, MeshTypeEnum::Mesh_LevelMesh, vertexList, indexList);
    }
}

void Level_DestroyLevel()
{
    spriteSystem.Destroy();
    for (auto& tileMap : levelArchive.LevelTileSetMap)
    {
        VRAM_DeleteLevelVRAM(tileMap.second.LevelTileListPtr);
    }

    for (auto& levelLayer : levelArchive.LevelLayerList)
    {
        Level2D_DeleteLevel(levelLayer.TileIdMap, levelLayer.TileMap, levelLayer.VertexList, levelLayer.IndexList);
    }
}