#include "pch.h"
#include "MemorySystem.h"
#include "VRAM.h"
#include "FileSystem.h"
#include "SpriteSystem.h"

VramSystem vramSystem = VramSystem();

VramSystem::VramSystem()
{
}

VramSystem::~VramSystem()
{
}

SpriteVram VramSystem::LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture)
{
    nlohmann::json json = fileSystem.LoadJsonFile(spritePath);
    ivec2 spritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] };
    ivec2 spriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y);
    ivec2 spriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] };

    return SpriteVram
    {
        .VramSpriteID = VkGuid(json["VramSpriteId"].get<String>().c_str()),
        .SpriteMaterialID = material.MaterialGuid,
        .SpriteLayer = json["SpriteLayer"],
        .SpriteColor = vec4{ json["SpriteColor"][0], json["SpriteColor"][1], json["SpriteColor"][2], json["SpriteColor"][3] },
        .SpritePixelSize = ivec2{ json["SpritePixelSize"][0], json["SpritePixelSize"][1] },
        .SpriteScale = ivec2{ json["SpriteScale"][0], json["SpriteScale"][1] },
        .SpriteCells = ivec2(texture.width / spritePixelSize.x, texture.height / spritePixelSize.y),
        .SpriteUVSize = vec2(1.0f / (float)spriteCells.x, 1.0f / (float)spriteCells.y),
        .SpriteSize = vec2(spritePixelSize.x * spriteScale.x, spritePixelSize.y * spriteScale.y),
    };
}

Vector<Animation2D> VramSystem::LoadSpriteAnimations(const char* spritePath)
{
    Vector<Animation2D> animationList;
    nlohmann::json json = fileSystem.LoadJsonFile(spritePath);
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
            .FrameList = spriteFrameList,
            .FrameHoldTime = json["AnimationList"][x]["FrameHoldTime"].get<float>(),
        };
        animationList.emplace_back(animation);
    }

    return animationList;
}

LevelTileSet VramSystem::LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture)
{
    nlohmann::json json = fileSystem.LoadJsonFile(tileSetPath);

    LevelTileSet tileSet = LevelTileSet();
    tileSet.TileSetId = VkGuid(json["TileSetId"].get<String>().c_str());
    tileSet.MaterialId = material.MaterialGuid;
    tileSet.TilePixelSize = ivec2{ json["TilePixelSize"][0], json["TilePixelSize"][1] };
    tileSet.TileSetBounds = ivec2{ tileVramTexture.width / tileSet.TilePixelSize.x,  tileVramTexture.height / tileSet.TilePixelSize.y };
    tileSet.TileUVSize = vec2(1.0f / (float)tileSet.TileSetBounds.x, 1.0f / (float)tileSet.TileSetBounds.y);

    return tileSet;
}

bool VramSystem::SpriteVramExists(const VkGuid& vramId)
{
    return std::any_of(spriteSystem.SpriteVramList.begin(), spriteSystem.SpriteVramList.end(), [vramId](const SpriteVram& sprite) { return sprite.VramSpriteID == vramId; });
}

void VramSystem::LoadTileSets(const char* tileSetPath, LevelTileSet& tileSet)
{
    nlohmann::json json = fileSystem.LoadJsonFile(tileSetPath);

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

LevelLayout VramSystem::LoadLevelInfo(const char* levelLayoutPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(levelLayoutPath);

    LevelLayout levelLayout;
    levelLayout.LevelLayoutId = VkGuid(json["LevelLayoutId"].get<String>().c_str());
    levelLayout.LevelBounds = ivec2(json["LevelBounds"][0], json["LevelBounds"][1]);
    levelLayout.TileSizeinPixels = ivec2(json["TileSizeInPixels"][0], json["TileSizeInPixels"][1]);
    return levelLayout;
}

Vector<Vector<uint>> VramSystem::LoadLevelLayout(const String& levelLayoutPath)
{
    Vector<Vector<uint>> levelLayerList;
    nlohmann::json json = fileSystem.LoadJsonFile(levelLayoutPath.c_str());
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
        levelLayerList.push_back(levelLayerMap);
    }
    return levelLayerList;
}

void VramSystem::DeleteSpriteVRAM(Animation2D* animationListPtr, ivec2* animationFrameListPtr)
{
    memorySystem.DeletePtr<Animation2D>(animationListPtr);
    memorySystem.DeletePtr<ivec2>(animationFrameListPtr);
}

void VramSystem::DeleteLevelVRAM(Tile* levelTileList)
{
    memorySystem.DeletePtr<Tile>(levelTileList);
}

void VramSystem::DeleteLevelLayerPtr(uint** levelLayerPtr)
{
    memorySystem.DeletePtr<uint*>(levelLayerPtr);
}

void VramSystem::DeleteLevelLayerMapPtr(uint* levelLayerMapPtr)
{
    memorySystem.DeletePtr<uint>(levelLayerMapPtr);
}
