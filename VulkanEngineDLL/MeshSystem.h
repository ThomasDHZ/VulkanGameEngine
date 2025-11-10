#pragma once
#include "Platform.h"
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



class MeshSystem
{
private:
	Vector<uint32>				 FreeMeshIndicesList;
	Vector<Mesh>                 MeshList;
	Vector<MeshPropertiesStruct> MeshPropertiesList;

	uint32 GetNextMeshIndex();
	void   UpdateMesh(Mesh& mesh, ShaderStruct& shaderStruct, VulkanBuffer& meshPropertiesBuffer, uint shaderMaterialBufferIndex, const float& deltaTime);

public:
	Vector<Vector<Vertex2D>>     Vertex2DList;
	Vector<Vector<uint>>         IndexList;

	MeshSystem();
	~MeshSystem();

	DLL_EXPORT uint CreateMesh(MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
	DLL_EXPORT void Update(const float& deltaTime);
	DLL_EXPORT void Destroy(uint meshId);
	DLL_EXPORT void DestroyAllGameObjects();
	DLL_EXPORT const Mesh& FindMesh(const uint& meshId);
	DLL_EXPORT const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
	DLL_EXPORT const Vector<Mesh>& FindMeshByVertexType(VertexTypeEnum vertexType);
};
DLL_EXPORT MeshSystem meshSystem;

#ifdef __cplusplus
extern "C" 
	{
		#endif
			DLL_EXPORT uint MeshSystem_CreateMesh(MeshTypeEnum meshType, Vertex2D* vertexListPtr, uint32* indexListPtr, size_t vertexListCount, size_t indexListCount);
			DLL_EXPORT void MeshSystem_Update( const float& deltaTime);
			DLL_EXPORT void MeshSystem_Destroy(uint meshId);
			DLL_EXPORT void MeshSystem_DestroyMesh(Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer);
			DLL_EXPORT void MeshSystem_DestroyAllGameObjects();
			DLL_EXPORT const Mesh& MeshSystem_FindMesh(const uint& meshId);
		#ifdef __cplusplus
	}
#endif