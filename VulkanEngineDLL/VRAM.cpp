#include "MemorySystem.h"
#include "VRAM.h"

SpriteVram VRAM_LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture)
{
    nlohmann::json json = Json::ReadJson(spritePath);
    ivec2 spritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] };
    ivec2 spriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y);
    ivec2 spriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] };
    
    return SpriteVram
    {
        .VramSpriteID = VkGuid(json["VramSpriteId"].get<String>().c_str()),
        .SpriteMaterialID = material.materialGuid,
        .SpriteLayer = json["SpriteLayer"],
        .SpriteColor = vec4{ json["SpriteColor"][0], json["SpriteColor"][1], json["SpriteColor"][2], json["SpriteColor"][3] },
        .SpritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] },
        .SpriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] },
        .SpriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y),
        .SpriteUVSize = vec2(1.0f / (float)spriteCells.x, 1.0f / (float)spriteCells.y),
        .SpriteSize = vec2(spritePixelSize.x * spriteScale.x, spritePixelSize.y * spriteScale.y),
    };
}

Animation2D* VRAM_LoadSpriteAnimations(const char* spritePath, size_t& animationListCount)
{
    Vector<Animation2D> animationList;
    nlohmann::json json = Json::ReadJson(spritePath);
    for (size_t x = 0; x < json["AnimationList"].size(); ++x)
    {
        Vector<ivec2> spriteFrameList;
        for (size_t y = 0; y < json["AnimationList"][x]["FrameList"].size(); ++y)
        {
            ivec2 frame =
            {
                json["AnimationList"][x]["FrameList"][y][0].get<float>(),
                json["AnimationList"][x]["FrameList"][y][1].get<float>()
            };
            spriteFrameList.emplace_back(frame);
        }

        Animation2D animation =
        {
            .AnimationId = json["AnimationList"][x]["AnimationId"].get<uint>(),
            .FrameList = memorySystem.AddPtrBuffer<ivec2>(spriteFrameList.data(), spriteFrameList.size(), __FILE__, __LINE__, __func__, ("Sprite animation frame list: " + String(spritePath)).c_str()),
            .FrameCount = json["AnimationList"][x]["FrameList"].size(),
            .FrameHoldTime = json["AnimationList"][x]["FrameHoldTime"].get<float>(),
        };
        animationList.emplace_back(animation);
    }

    animationListCount = animationList.size();
    Animation2D* animationListPtr = memorySystem.AddPtrBuffer<Animation2D>(animationList.data(), animationList.size(), __FILE__, __LINE__, __func__, ("Sprite animation: " + String(spritePath)).c_str());
    return animationListPtr;
}

LevelTileSet VRAM_LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture)
{
    nlohmann::json json = Json::ReadJson(tileSetPath);

    LevelTileSet tileSet = LevelTileSet();
    tileSet.TileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    tileSet.MaterialId = material.materialGuid;
    tileSet.TilePixelSize = ivec2{ json["TilePixelSize"][0], json["TilePixelSize"][1] };
    tileSet.TileSetBounds = ivec2{ tileVramTexture.width / tileSet.TilePixelSize.x,  tileVramTexture.height / tileSet.TilePixelSize.y };
    tileSet.TileUVSize = vec2(1.0f / (float)tileSet.TileSetBounds.x, 1.0f / (float)tileSet.TileSetBounds.y);

    return tileSet;
}

void VRAM_LoadTileSets(const char* tileSetPath, LevelTileSet& tileSet)
{
    nlohmann::json json = Json::ReadJson(tileSetPath);

    Vector<Tile> tileList;
    for (int x = 0; x < json["TileList"].size(); x++)
    {
        Tile tile;
        tile.TileId = json["TileList"][x]["TileId"];
        tile.TileUVCellOffset = ivec2(json["TileList"][x]["TileUVCellOffset"][0], json["TileList"][x]["TileUVCellOffset"][1]);
        tile.TileLayer = json["TileList"][x]["TileLayer"];
        tile.IsAnimatedTile = json["TileList"][x]["IsAnimatedTile"];
        tile.TileUVOffset = vec2(tile.TileUVCellOffset.x * tileSet.TileUVSize.x, tile.TileUVCellOffset.y * tileSet.TileUVSize.y);
        tileList.emplace_back(tile);
    }
    tileSet.LevelTileCount = tileList.size();

    tileSet.LevelTileListPtr = memorySystem.AddPtrBuffer<Tile>(tileList.size(), __FILE__, __LINE__, __func__);
    std::memcpy(tileSet.LevelTileListPtr, tileList.data(), tileList.size() * sizeof(Tile));
}

LevelLayout VRAM_LoadLevelInfo(const char* levelLayoutPath)
{
    nlohmann::json json = Json::ReadJson(levelLayoutPath);

    LevelLayout levelLayout;
    levelLayout.LevelLayoutId = VkGuid(json["LevelLayoutId"].get<String>().c_str());
    levelLayout.LevelBounds = ivec2(json["LevelBounds"][0], json["LevelBounds"][1]);
    levelLayout.TileSizeinPixels = ivec2(json["TileSizeInPixels"][0], json["TileSizeInPixels"][1]);
    return levelLayout;
}

uint** VRAM_LoadLevelLayout(const char* levelLayoutPath, size_t& levelLayerCount, size_t& levelLayerMapCount)
{
    nlohmann::json json = Json::ReadJson(levelLayoutPath);
    Vector<uint*> levelLayerList;
    for (int x = 0; x < json["LevelLayouts"].size(); x++)
    {
        Vector<uint> levelLayerMap;
        for (int y = 0; y < json["LevelLayouts"][x].size(); y++)
        {
            for (int z = 0; z < json["LevelLayouts"][x][y].size(); z++)
            {
                levelLayerMap.push_back(json["LevelLayouts"][x][y][z]);
            }
        }
        levelLayerMapCount = levelLayerMap.size();
        uint* levelLayerMapPtr = memorySystem.AddPtrBuffer<uint>(levelLayerMap.size(), __FILE__, __LINE__, __func__);
        std::memcpy(levelLayerMapPtr, levelLayerMap.data(), levelLayerMap.size() * sizeof(uint));
        levelLayerList.push_back(levelLayerMapPtr);
    }
    levelLayerCount = levelLayerList.size();
    uint** levelLayerListPtr = memorySystem.AddPtrBuffer<uint*>(levelLayerList.size(), __FILE__, __LINE__, __func__);
    std::memcpy(levelLayerListPtr, levelLayerList.data(), levelLayerList.size() * sizeof(uint*));
    return levelLayerListPtr;
}

void VRAM_DeleteSpriteVRAM(Animation2D* animationListPtr, ivec2* animationFrameListPtr)
{
    memorySystem.RemovePtrBuffer<Animation2D>(animationListPtr);
    memorySystem.RemovePtrBuffer<ivec2>(animationFrameListPtr);
}

void VRAM_DeleteLevelVRAM(Tile* levelTileList)
{
    memorySystem.RemovePtrBuffer<Tile>(levelTileList);
}

void VRAM_DeleteLevelLayerPtr(uint** levelLayerPtr)
{
    memorySystem.RemovePtrBuffer<uint*>(levelLayerPtr);
}

void VRAM_DeleteLevelLayerMapPtr(uint* levelLayerMapPtr)
{
    memorySystem.RemovePtrBuffer<uint>(levelLayerMapPtr);
}
