#pragma once

#include "Vertex.h"
#include <Mesh.h>
#include <BufferSystem.h>
#include "GameObjectSystem.h"
#include "MaterialSystem.h"
#include "LevelSystem.h"
#include "GameSystem.h"

class MeshSystem
{
private:
    static uint NextMeshId;
    static uint NextSpriteMeshId;
    static uint NextLevelLayerMeshId;

    UnorderedMap<uint, Mesh>                           MeshMap;
    UnorderedMap<UM_SpriteBatchID, Mesh>               SpriteMeshMap;
    UnorderedMap<LevelGuid, Vector<Mesh>>              LevelLayerMeshListMap;
    UnorderedMap<uint, Vector<Vertex2D>>               Vertex2DListMap;
    UnorderedMap<uint, Vector<uint>>                   IndexListMap;

public:
    MeshSystem();
    ~MeshSystem();

	template<class T>
	uint CreateMesh(Vector<T>& vertexList, Vector<uint32>& indexList, VkGuid materialId)
	{
		uint meshId = NextMeshId++;
		mat4 meshMatrix = mat4(1.0f);
		Vertex2DListMap[meshId] = vertexList;
		IndexListMap[meshId] = indexList;

		MeshLoader meshLoader =
		{
			.ParentGameObjectID = 0,
			.VertexType = BufferTypeEnum::BufferType_Vector2D,
			.MeshId = meshId,
			.MaterialId = materialId,
			.VertexLoader = VertexLoaderStruct
			{
				.MeshVertexBufferId = static_cast<uint32>(++NextBufferId),
				.SizeofVertex = sizeof(T),
				.VertexCount = static_cast<uint32>(vertexList.size()),
				.VertexData = static_cast<void*>(vertexList.data()),
			},
			.IndexLoader = IndexLoaderStruct
			{
				.MeshIndexBufferId = static_cast<uint32>(++NextBufferId),
				.SizeofIndex = sizeof(uint),
				.IndexCount = static_cast<uint32>(indexList.size()),
				.IndexData = static_cast<void*>(indexList.data()),
			},
			.TransformLoader = TransformLoaderStruct
			{
				.MeshTransformBufferId = static_cast<uint32>(++NextBufferId),
				.SizeofTransform = sizeof(mat4),
				.TransformData = static_cast<void*>(&meshMatrix),
			},
			.MeshPropertiesLoader = MeshPropertiesLoaderStruct
			{
				.PropertiesBufferId = static_cast<uint32>(++NextBufferId),
				.SizeofMeshProperties = sizeof(MeshPropertiesStruct),
				.MeshPropertiesData = static_cast<void*>(&MeshMap[meshId].MeshProperties)
			}
		};

		Mesh mesh = Mesh_CreateMesh(renderSystem.renderer, meshLoader, bufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId],
			bufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId],
			bufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId],
			bufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId]);
		return meshId;
	}

    uint CreateSpriteLayerMesh(Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
    uint CreateLevelLayerMesh(const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);

    void Update(const float& deltaTime);

    void Destroy(uint meshId);
    void DestroyAllGameObjects();

    const Mesh& FindMesh(const uint& id);
    const Mesh& FindSpriteMesh(const uint& id);
    const Vector<Mesh>& FindLevelLayerMeshList(const LevelGuid& guid);
    const Vector<Vertex2D>& FindVertex2DList(const uint& id);
    const Vector<uint>& FindIndexList(const uint& id);

    const Vector<Mesh> MeshList();
    const Vector<Mesh> SpriteMeshList();
};

extern MeshSystem meshSystem;