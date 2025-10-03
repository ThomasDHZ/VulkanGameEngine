#pragma once
#include <vulkan/vulkan_core.h>
#include "DLL.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "vertex.h"


DLL_EXPORT uint NextMeshId;
DLL_EXPORT uint NextSpriteMeshId;
DLL_EXPORT uint NextLevelLayerMeshId;

struct MeshPropertiesStruct
{
	alignas(8)  size_t ShaderMaterialBufferIndex = 0;
	alignas(16) mat4   MeshTransform = mat4(1.0f);
};

const VkBufferUsageFlags MeshBufferUsageSettings = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT|
VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
VK_BUFFER_USAGE_TRANSFER_DST_BIT;

const VkMemoryPropertyFlags MeshBufferPropertySettings = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

struct VertexLoaderStruct
{
	BufferTypeEnum VertexType;
	uint32 MeshVertexBufferId;
	size_t SizeofVertex;
	size_t VertexCount;
	void*  VertexData;
};

struct IndexLoaderStruct
{
	uint32 MeshIndexBufferId;
	size_t SizeofIndex;
	size_t IndexCount;
	void*  IndexData;
};

struct TransformLoaderStruct
{
	uint32 MeshTransformBufferId;
	size_t SizeofTransform;
	void*  TransformData;
};

struct MeshPropertiesLoaderStruct
{
	uint32 PropertiesBufferId;
	size_t SizeofMeshProperties;
	void*  MeshPropertiesData;
};

struct MeshLoader
{
	uint ParentGameObjectID;
	uint MeshId;
	VkGuid MaterialId;

	VertexLoaderStruct VertexLoader;
	IndexLoaderStruct IndexLoader;
	TransformLoaderStruct TransformLoader;
	MeshPropertiesLoaderStruct MeshPropertiesLoader;
};

struct Mesh
{
	uint32 MeshId = 0;
	uint32 ParentGameObjectID = 0;
	uint32 GameObjectTransform = 0;
	size_t VertexCount = 0;
	size_t IndexCount = 0;
	VkGuid MaterialId;

	BufferTypeEnum VertexType;
	vec3 MeshPosition = vec3(0.0f);
	vec3 MeshRotation = vec3(0.0f);
	vec3 MeshScale = vec3(1.0f);

	int	MeshVertexBufferId;
	int	MeshIndexBufferId;
	int MeshTransformBufferId;
	int	PropertiesBufferId;

	MeshPropertiesStruct MeshProperties;
};

struct MeshArchive
{
	UnorderedMap<uint, Mesh>                           MeshMap;
	UnorderedMap<UM_SpriteBatchID, Mesh>               SpriteMeshMap;
	UnorderedMap<LevelGuid, Vector<Mesh>>              LevelLayerMeshListMap;
	UnorderedMap<uint, Vector<Vertex2D>>               Vertex2DListMap;
	UnorderedMap<uint, Vector<uint>>                   IndexListMap;
};
DLL_EXPORT MeshArchive meshArchive;

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT Mesh Mesh_CreateMesh(const GraphicsRenderer& renderer, const MeshLoader& meshLoader, VulkanBuffer& outVertexBuffer, VulkanBuffer& outIndexBuffer, VulkanBuffer& outTransformBuffer, VulkanBuffer& outPropertiesBuffer);
DLL_EXPORT void Mesh_UpdateMesh(const GraphicsRenderer& renderer, Mesh& mesh, ShaderStruct& shaderStruct, VulkanBuffer& meshPropertiesBuffer, uint shaderMaterialBufferIndex, const float& deltaTime);
DLL_EXPORT void Mesh_DestroyMesh(const GraphicsRenderer& renderer, Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer);
DLL_EXPORT uint Mesh_CreateSpriteLayerMesh(const GraphicsRenderer& renderer, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
DLL_EXPORT uint Mesh_CreateLevelLayerMesh(const GraphicsRenderer& renderer, const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
DLL_EXPORT void Mesh_Update(const GraphicsRenderer& renderer, const float& deltaTime);
DLL_EXPORT void Mesh_Destroy(const GraphicsRenderer& renderer, uint meshId);
DLL_EXPORT void Mesh_DestroyAllGameObjects(const GraphicsRenderer& renderer);
DLL_EXPORT const Mesh& Mesh_FindMesh(const uint& id);
DLL_EXPORT const Mesh& Mesh_FindSpriteMesh(const uint& id);

int Mesh_CreateVertexBuffer(const GraphicsRenderer& renderer, const VertexLoaderStruct& vertexLoader, VulkanBuffer& outVertexBuffer);
int Mesh_CreateIndexBuffer(const GraphicsRenderer& renderer, const IndexLoaderStruct& indexLoader, VulkanBuffer& outIndexBuffer);
int Mesh_CreateTransformBuffer(const GraphicsRenderer& renderer, const TransformLoaderStruct& transformLoader, VulkanBuffer& outTransformBuffer);
int Mesh_CreateMeshPropertiesBuffer(const GraphicsRenderer& renderer, const MeshPropertiesLoaderStruct& meshProperties, VulkanBuffer& outPropertiesBufferId);
#ifdef __cplusplus
}
#endif
DLL_EXPORT const Vector<Mesh>& Mesh_FindLevelLayerMeshList(const LevelGuid& guid);
DLL_EXPORT const Vector<Vertex2D>& Mesh_FindVertex2DList(const uint& id);
DLL_EXPORT const Vector<uint>& Mesh_FindIndexList(const uint& id);
DLL_EXPORT const Vector<Mesh> Mesh_MeshList();
DLL_EXPORT const Vector<Mesh> Mesh_SpriteMeshList();