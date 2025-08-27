#pragma once
#include <vulkan/vulkan_core.h>
#include "DLL.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"

struct MeshPropertiesStruct
{
	alignas(4)  uint32 ShaderMaterialBufferIndex = 0;
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

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT Mesh Mesh_CreateMesh(const GraphicsRenderer& renderer, const MeshLoader& meshLoader, VulkanBuffer& outVertexBuffer, VulkanBuffer& outIndexBuffer, VulkanBuffer& outTransformBuffer, VulkanBuffer& outPropertiesBuffer);
DLL_EXPORT void Mesh_UpdateMesh(const GraphicsRenderer& renderer, Mesh& mesh, VulkanBuffer& meshPropertiesBuffer, uint32 shaderMaterialBufferIndex, const float& deltaTime);
//DLL_EXPORT void Mesh_UpdateVertices()
DLL_EXPORT void Mesh_DestroyMesh(const GraphicsRenderer& renderer, Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer);

int Mesh_CreateVertexBuffer(const GraphicsRenderer& renderer, const VertexLoaderStruct& vertexLoader, VulkanBuffer& outVertexBuffer);
int Mesh_CreateIndexBuffer(const GraphicsRenderer& renderer, const IndexLoaderStruct& indexLoader, VulkanBuffer& outIndexBuffer);
int Mesh_CreateTransformBuffer(const GraphicsRenderer& renderer, const TransformLoaderStruct& transformLoader, VulkanBuffer& outTransformBuffer);
int Mesh_CreateMeshPropertiesBuffer(const GraphicsRenderer& renderer, const MeshPropertiesLoaderStruct& meshProperties, VulkanBuffer& outPropertiesBufferId);
#ifdef __cplusplus
}
#endif