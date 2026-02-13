#pragma once
#include "Platform.h"
#include "ShaderSystem.h"

struct MeshPropertiesStruct
{
	alignas(8)  size_t ShaderMaterialBufferIndex = 0;
	alignas(16) mat4   MeshTransform = mat4(1.0f);
};

enum MeshTypeEnum
{
	kMesh_SpriteMesh,
	kMesh_LevelMesh,
	kMesh_SkyBoxMesh,
	kMesh_LineMesh,
	kMesh_Undefined
};

enum VertexLayoutEnum
{
	kVertexLayout_NullVertex,
	kVertexLayout_Vertex2D,
	kVertexLayout_SpriteInstanceVertex,
	kVertexLayout_SkyBoxVertex,
	kVertexLayout_Undefined
};

struct VertexLayout
{
	VertexLayoutEnum VertexType = kVertexLayout_NullVertex;
	uint64 VertexDataSize = UINT64_MAX;
	void* VertexData = nullptr;
};

struct NullVertexLayout
{

};

struct Vertex2DLayout
{
	vec2 Position = vec2(0.0f);
	vec2 UV = vec2(0.0f);

	Vertex2DLayout()
	{
		Position = vec2(0.0f);
		UV = vec2(0.0f);
	}

	Vertex2DLayout(vec2 position, vec2 uv)
	{
		Position = position;
		UV = uv;
	}
};

struct SkyboxVertexLayout
{
	vec3 Position = glm::vec3(0.0f);
};

struct MeshAssetData
{
	uint32 VertexBufferId = UINT32_MAX;
	uint32 IndexBufferId = UINT32_MAX;
	uint32 IndexCount = 0;
	VertexLayoutEnum Layout = VertexLayoutEnum::kVertexLayout_Undefined;
};

struct Mesh
{
	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint32 MeshShaderBufferIndex = UINT32_MAX;
	MeshTypeEnum MeshTypeId = kMesh_Undefined;
	MeshAssetData MeshData;
	VertexLayoutEnum VertexLayout = kVertexLayout_NullVertex;
	uint32 MeshPropertiesId = UINT32_MAX;
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
		Vector<VertexLayout>		 VertexList;
		Vector<Vector<uint>>         IndexList;
		VulkanBuffer 				 MeshPropertiesBuffer;
		VulkanBuffer				 TransformBuffer;

		DLL_EXPORT void MeshSystemStartUp();
		DLL_EXPORT uint CreateMesh(MeshTypeEnum meshtype, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId = VkGuid());
		DLL_EXPORT void Update(const float& deltaTime);
		DLL_EXPORT void Destroy(uint meshId);
		DLL_EXPORT void DestroyAllGameObjects();
		DLL_EXPORT const Mesh& FindMesh(const uint& meshId);
		DLL_EXPORT const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
		DLL_EXPORT const Vector<Mesh>& FindMeshByVertexType(VertexLayoutEnum vertexType);
};
extern DLL_EXPORT MeshSystem& meshSystem;
inline MeshSystem& MeshSystem::Get()
{
	static MeshSystem instance;
	return instance;
}