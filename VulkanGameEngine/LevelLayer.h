//#pragma once
//#include "Typedef.h"
//#include "Vertex.h"
//#include "Mesh.h"
//#include "VkGuid.h"
//#include <VRAM.h>
//
//class LevelLayer
//{
//private:
//
//	void LoadLevelMesh();
//
//public:
//	VkGuid				LevelId;
//	uint				MeshId;
//	VkGuid				MaterialId;
//	VkGuid				TileSetId;
//	int					LevelLayerIndex;
//	ivec2				LevelBounds;
//	Vector<uint>		TileIdMap;
//	Vector<Tile>		TileMap;
//	Vector<Vertex2D>	VertexList;
//	Vector<uint32>		IndexList;
//
//	LevelLayer();
//	LevelLayer(VkGuid& levelId, VkGuid& tileSetId, Vector<uint>& tileIdMap, ivec2& levelBounds, int levelLayerIndex);
//	~LevelLayer();
//
//	void Update(const float& deltaTime);
//};
//
