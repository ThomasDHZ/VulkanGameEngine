#pragma once
#include "Platform.h"
#include "ShaderSystem.h"
#include "Vertex.h"

struct MeshPropertiesStruct
{
	alignas(8)  size_t ShaderMaterialBufferIndex = 0;
	alignas(16) mat4   MeshTransform = mat4(1.0f);
};

struct MeshDLL;
struct Mesh
{
	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint32 MeshShaderBufferIndex = UINT32_MAX;
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
public:
	static MeshSystem& Get();

private:
	MeshSystem() = default;
	~MeshSystem() = default;
	MeshSystem(const MeshSystem&) = delete;
	MeshSystem& operator=(const MeshSystem&) = delete;
	MeshSystem(MeshSystem&&) = delete;
	MeshSystem& operator=(MeshSystem&&) = delete;

		Vector<uint32>				 FreeMeshIndicesList;
		Vector<MeshPropertiesStruct> MeshPropertiesList;

		uint32 GetNextMeshIndex();
		void   UpdateMesh(Mesh& mesh, const float& deltaTime);

	public:
		Vector<Mesh>                 MeshList;
		Vector<Vector<Vertex2D>>     Vertex2DList;
		Vector<Vector<uint>>         IndexList;

		DLL_EXPORT uint CreateMesh(MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList);
		DLL_EXPORT uint CreateMesh(MeshTypeEnum meshtype, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList, VkGuid& materialId);
		DLL_EXPORT void Update(const float& deltaTime);
		DLL_EXPORT void Destroy(uint meshId);
		DLL_EXPORT void DestroyAllGameObjects();
		DLL_EXPORT const Mesh& FindMesh(const uint& meshId);
		DLL_EXPORT const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
		DLL_EXPORT const Vector<Mesh>& FindMeshByVertexType(VertexTypeEnum vertexType);
};
extern DLL_EXPORT MeshSystem& meshSystem;
inline MeshSystem& MeshSystem::Get()
{
	static MeshSystem instance;
	return instance;
}