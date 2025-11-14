#pragma once
#include <Platform.h>
#include <MaterialSystem.h>
#include <TextureSystem.h>

struct SpriteVram
{
    VkGuid VramSpriteID = VkGuid();
    VkGuid SpriteMaterialID = VkGuid();
    uint SpriteLayer = 0;
    vec4 SpriteColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ivec2 SpritePixelSize = ivec2();
    vec2 SpriteScale = vec2(1.0f, 1.0f);
    ivec2 SpriteCells = ivec2(0, 0);
    vec2 SpriteUVSize = vec2();
    vec2 SpriteSize = vec2(50.0f);
    uint AnimationListID = 0;
};

struct Tile
{
    uint  TileId = 0;
    ivec2 TileUVCellOffset = ivec2();
    vec2  TileUVOffset = vec2();
    int	  TileLayer = 0;
    bool  IsAnimatedTile = false;
};

struct LevelTileSet
{
    VkGuid			  TileSetId = VkGuid();
    VkGuid			  MaterialId = VkGuid();
    vec2			  TilePixelSize = vec2();
    ivec2			  TileSetBounds = ivec2();
    vec2			  TileScale = vec2(5.0f);
    vec2			  TileUVSize = vec2();
    Tile*             LevelTileListPtr = nullptr;
    size_t            LevelTileCount = 0;
};

struct LevelLayout
{
    VkGuid					  LevelLayoutId;
    ivec2					  LevelBounds;
    ivec2					  TileSizeinPixels;
};

struct Animation2D
{
    uint          AnimationId;
    Vector<ivec2> FrameList;
    float         FrameHoldTime;
};

struct Animation2DDLL
{
    uint   AnimationId;
    ivec2* FrameList;
    size_t FrameCount;
    float  FrameHoldTime;
};

typedef Vector<ivec2> AnimationFrames;

class VramSystem
{
public:
    VramSystem();
    ~VramSystem();

    DLL_EXPORT SpriteVram           LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture);
    DLL_EXPORT Vector<Animation2D>  LoadSpriteAnimations(const char* spritePath);
    DLL_EXPORT LevelTileSet         LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture);
    DLL_EXPORT bool                 SpriteVramExists(const VkGuid& vramId);
    DLL_EXPORT void                 LoadTileSets(const char* tileSetPath, LevelTileSet& levelTileSet);
    DLL_EXPORT LevelLayout          LoadLevelInfo(const char* levelLayoutPath);
    DLL_EXPORT Vector<Vector<uint>> LoadLevelLayout(const String& levelLayoutPath);
    DLL_EXPORT void                 DeleteSpriteVRAM(Animation2D* animationListPtr, ivec2* animationFrameListPtr);
    DLL_EXPORT void                 DeleteLevelVRAM(Tile* levelTileList);
    DLL_EXPORT void                 DeleteLevelLayerPtr(uint** levelLayerPtr);
    DLL_EXPORT void                 DeleteLevelLayerMapPtr(uint* levelLayerMapPtr);
};
DLL_EXPORT VramSystem vramSystem;

//#ifdef __cplusplus
//    extern "C" {
//#endif
//        DLL_EXPORT SpriteVram VRAM_LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture);
//        DLL_EXPORT Animation2D* VRAM_LoadSpriteAnimations(const char* spritePath, size_t& animationListCount);
//        DLL_EXPORT LevelTileSet VRAM_LoadTileSetVRAM(const char* tileSetPath, const Material& material, const Texture& tileVramTexture);
//        DLL_EXPORT bool VRAM_SpriteVramExists(const VkGuid& vramId);
//        DLL_EXPORT void VRAM_LoadTileSets(const char* tileSetPath, LevelTileSet& levelTileSet);
//        DLL_EXPORT LevelLayout VRAM_LoadLevelInfo(const char* levelLayoutPath);
//        DLL_EXPORT uint** VRAM_LoadLevelLayout(const char* levelLayoutPath, size_t& levelLayerCount, size_t& levelLayerMapCount);
//        DLL_EXPORT void VRAM_DeleteSpriteVRAM(Animation2D* animationListPtr, ivec2* animationFrameListPtr);
//        DLL_EXPORT void VRAM_DeleteLevelVRAM(Tile* levelTileList);
//        DLL_EXPORT void VRAM_DeleteLevelLayerPtr(uint** levelLayerPtr);
//        DLL_EXPORT void VRAM_DeleteLevelLayerMapPtr(uint* levelLayerMapPtr);
//#ifdef __cplusplus
//    }
//#endif