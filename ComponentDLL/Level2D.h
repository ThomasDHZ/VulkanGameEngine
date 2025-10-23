#pragma once
#include "DLL.h"
#include "Typedef.h"
#include <MeshSystem.h>
#include "VkGuid.h"
#include "VRAM.h"
#include <Vertex.h>
#include "SpriteSystem.h"

struct LevelLayer
{
	VkGuid				LevelId;
	uint				MeshId;
	VkGuid				MaterialId;
	VkGuid				TileSetId;
	int					LevelLayerIndex;
	ivec2				LevelBounds;
	uint*				TileIdMap;
	Tile*				TileMap;
	Vertex2D*			VertexList;
	uint32*				IndexList;
	size_t				TileIdMapCount;
	size_t				TileMapCount;
	size_t				VertexListCount;
	size_t				IndexListCount;
};

struct LevelArchive
{
    LevelLayout levelLayout;
    Vector<LevelLayer> LevelLayerList;
    Vector<Vector<uint>> LevelTileMapList;
    UnorderedMap<RenderPassGuid, LevelTileSet> LevelTileSetMap;
};
DLL_EXPORT LevelArchive levelArchive;

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT LevelLayer Level2D_LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
	DLL_EXPORT void Level2D_DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList);
	DLL_EXPORT VkGuid Level_LoadTileSetVRAM(const char* tileSetPath);
	DLL_EXPORT void Level_LoadLevelLayout(const char* levelLayoutPath);
	DLL_EXPORT void Level_LoadLevelMesh(VkGuid& tileSetId);
	DLL_EXPORT void Level_DestroyLevel();
#ifdef __cplusplus
}
#endif
