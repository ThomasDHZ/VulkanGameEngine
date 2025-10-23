#pragma once
#include <vulkan/vulkan_core.h>
#include <math.h>
#include "DLL.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "VulkanBuffer.h"
#include "VulkanShaderSystem.h"
#include "vertex.h"

struct MeshPropertiesStruct
{
	alignas(8)  size_t ShaderMaterialBufferIndex = 0;
	alignas(16) mat4   MeshTransform = mat4(1.0f);
};

struct Mesh
{
	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint32 MeshTypeId = UINT32_MAX;
	uint32 VertexTypeId = UINT32_MAX;
	uint32 MeshPropertiesId = UINT32_MAX;
	uint32 MeshVertexBufferId = UINT32_MAX;
	uint32 MeshIndexBufferId = UINT32_MAX;
	uint32 MeshTransformBufferId = UINT32_MAX;
	uint32 PropertiesBufferId = UINT32_MAX;
	uint32 VertexIndex = UINT32_MAX;
	uint32 IndexIndex = UINT32_MAX;
	vec3 MeshPosition = vec3(0.0f);
	vec3 MeshRotation = vec3(0.0f);
	vec3 MeshScale = vec3(1.0f);
	VkGuid MaterialId = VkGuid();
	void* MeshExtension = nullptr;
};

#ifdef __cplusplus
extern "C" 
	{
		#endif
			DLL_EXPORT uint32 GetNextMeshIndex();
			DLL_EXPORT uint Mesh_CreateMesh(const GraphicsRenderer& renderer, MeshTypeEnum meshType, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
			DLL_EXPORT void Mesh_Update(const GraphicsRenderer& renderer, const float& deltaTime);
			DLL_EXPORT void Mesh_UpdateMesh(const GraphicsRenderer& renderer, Mesh& mesh, ShaderStruct& shaderStruct, VulkanBuffer& meshPropertiesBuffer, uint shaderMaterialBufferIndex, const float& deltaTime);
			DLL_EXPORT void Mesh_Destroy(const GraphicsRenderer& renderer, uint meshId);
			DLL_EXPORT void Mesh_DestroyMesh(const GraphicsRenderer& renderer, Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer);
			DLL_EXPORT void Mesh_DestroyAllGameObjects(const GraphicsRenderer& renderer);
			DLL_EXPORT const Mesh& Mesh_FindMesh(const uint& id);
		#ifdef __cplusplus
	}
#endif

DLL_EXPORT const Vector<Mesh> Mesh_FindMeshByMeshType(MeshTypeEnum meshType);
DLL_EXPORT const Vector<Mesh>& Mesh_FindMeshByVertexType(VertexTypeEnum vertexType);
DLL_EXPORT const Vector<Mesh>& Mesh_FindMeshList(const uint32& meshTypeId);

class MeshSystem
{
private:
public:
	Vector<uint32>				 FreeMeshIndicesList;
	Vector<Mesh>                 MeshList;
	Vector<MeshPropertiesStruct> MeshPropertiesList;
	Vector<Vector<Vertex2D>>     Vertex2DList;
	Vector<Vector<uint>>         IndexList;

	MeshSystem() { }
	~MeshSystem() { }

	uint CreateSpriteLayerMesh(const GraphicsRenderer& renderer, MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList) { return Mesh_CreateMesh(renderer, meshtype, vertexList, indexList); }
	void Update(const float& deltaTime) { Mesh_Update(renderer, deltaTime); }
	void Destroy(uint meshId) { Mesh_Destroy(renderer, meshId); }
	void DestroyAllGameObjects() { Mesh_DestroyAllGameObjects(renderer); }
	const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType) { return Mesh_FindMeshByMeshType(meshType); }
	const Vector<Mesh>& FindMeshByVertexType(VertexTypeEnum vertexType) { return Mesh_FindMeshByVertexType(vertexType); }
};
DLL_EXPORT MeshSystem meshSystem;