#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "Mesh.h"
#include "VkGuid.h"
#include "VRAM.h"
#include <Vertex.h>

struct LevelLayer
{
	VkGuid				LevelId;
	uint				MeshId;
	VkGuid				MaterialId;
	VkGuid				TileSetId;
	int					LevelLayerIndex;
	ivec2				LevelBounds;
	uint* TileIdMap;
	Tile* TileMap;
	Vertex2D* VertexList;
	uint32* IndexList;
	size_t				TileIdMapCount;
	size_t				TileMapCount;
	size_t				VertexListCount;
	size_t				IndexListCount;
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT LevelLayer Level2D_LoadLevelInfo(VkGuid& levelId, const LevelTileSet& tileSet, uint* tileIdMap, size_t tileIdMapCount, ivec2& levelBounds, int levelLayerIndex);
	DLL_EXPORT void Level2D_DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint32* IndexList);
#ifdef __cplusplus
}
#endif