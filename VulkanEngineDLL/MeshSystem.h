#pragma once
#include "Platform.h"
#include "ShaderSystem.h"
#include "MemoryPool.h"
#include "MemoryPoolSystem.h"
#include <xxhash.h>

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
	Vector<uint32> MeshIdUsageList = Vector<uint32>();
	uint32 VertexBufferId = UINT32_MAX;
	uint32 IndexBufferId = UINT32_MAX;
	uint32 IndexCount = UINT32_MAX;
	VertexLayoutEnum Layout = VertexLayoutEnum::kVertexLayout_Undefined;
};

struct VertexLayout
{
	VertexLayoutEnum VertexType = kVertexLayout_Undefined;
	uint64 VertexDataSize = UINT64_MAX;
	void* VertexData = nullptr;
};

struct alignas(4) MeshBufferHeader
{
	uint MeshOffset;
	uint MeshCount;
	uint MeshSize;
};

struct MeshPropertiesStruct
{
	uint   MaterialIndex;
	mat4   MeshTransform;
};

struct Mesh
{
	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint64 SharedAssetId = UINT64_MAX;
	uint32 ObjectDataIndex = UINT32_MAX;
	MeshTypeEnum Type = MeshTypeEnum::kMesh_Undefined;
	VertexLayoutEnum Layout = VertexLayoutEnum::kVertexLayout_Undefined;
	vec3 Position = vec3(0.0f);
	vec3 Rotation = vec3(0.0f);
	vec3 Scale = vec3(1.0f);
	VkGuid MaterialId;
	bool IsTransformDirty = true;
	bool IsMaterialDirty = true;
	void* Extension = nullptr;
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

	Vector<uint32> FreeMeshIds;
	UnorderedMap<uint64, uint32> MeshAssetLookup;
	Vector<MeshAssetData> MeshAssetDataList;
	uint32 GetNextMeshId();
	uint64 HashAssetKey(std::string_view key);

public:
	Vector<Mesh> MeshList;

	DLL_EXPORT void StartUp();
	DLL_EXPORT uint CreateMesh(const String& key, MeshTypeEnum meshtype, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId = VkGuid());
	DLL_EXPORT void Update(const float& deltaTime);
	DLL_EXPORT void Destroy(uint meshId);
	DLL_EXPORT void DestroyAllGameObjects();
	DLL_EXPORT const Mesh& FindMesh(const uint& meshId);
	DLL_EXPORT MeshAssetData& FindMeshAssetData(const uint64& meshAssetId);
	DLL_EXPORT const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
};
extern DLL_EXPORT MeshSystem& meshSystem;
inline MeshSystem& MeshSystem::Get()
{
	static MeshSystem instance;
	return instance;
}