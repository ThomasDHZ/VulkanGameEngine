#pragma once

#include <Platform.h>
#include "ShaderSystem.h"
#include "MemoryPoolSystem.h"
#include "RenderSystem.h"
#include <xxhash.h>
#include <entt/entt.hpp>
#include <VulkanMesh.h>

enum VertexLayoutEnum
{
	kVertexLayout_NullVertex,
	kVertexLayout_Vertex2D,
	kVertexLayout_SpriteInstanceVertex,
	kVertexLayout_SkyBoxVertex,
	kVertexLayout_LineVertex,
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

struct TextVertex2DLayout
{
	vec2 Position = vec2(0.0f);
	vec2 UV = vec2(0.0f);
	vec3 Color = vec3(1.0f);

	TextVertex2DLayout()
	{
		Position = vec2(0.0f);
		UV = vec2(0.0f);
		Color = vec3(1.0f);
	}

	TextVertex2DLayout(vec2 position, vec2 uv, vec3 color)
	{
		Position = position;
		UV = uv;
		Color = color;
	}
};

struct LineVertex2DLayout
{
	vec2 Position = vec2(0.0f);
	vec4 Color = vec4(0.0f);

	LineVertex2DLayout()
	{
		Position = vec2(0.0f);
		Color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	
	LineVertex2DLayout(vec2 position, vec4 color)
	{
		Position = position;
		Color = color;
	}
};

struct LineVertex3DLayout
{
	vec3 Position = vec3(0.0f);
	vec4 Color = vec4(0.0f);

	LineVertex3DLayout()
	{
		Position = vec3(0.0f);
		Color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	LineVertex3DLayout(vec3 position, vec4 color)
	{
		Position = position;
		Color = color;
	}
};

struct SkyboxVertexLayout
{
	vec3 Position = glm::vec3(0.0f);
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

struct SpriteLayer
{
	uint32 InstanceCount = 0;
	uint32 StartInstanceIndex = 0;
	uint32 SpriteDrawLayer = UINT32_MAX;
};

struct MeshAssetData
{
	Vector<uint32> MeshIdUsageList = Vector<uint32>();
	uint32 VertexBufferId = UINT32_MAX;
	uint32 IndexBufferId = UINT32_MAX;
	uint32 VertexCount = UINT32_MAX;
	uint32 IndexCount = UINT32_MAX;
};

struct Mesh
{
	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint64 SharedAssetId = UINT64_MAX;
	uint32 ObjectDataIndex = UINT32_MAX;
	MeshTypeEnum Type = MeshTypeEnum::kMesh_Undefined;
	vec3 Position = vec3(0.0f);
	vec3 Rotation = vec3(0.0f);
	vec3 Scale = vec3(1.0f);
	VkGuid MaterialId;
	bool IsTransformDirty = true;
	bool IsMaterialDirty = true;
};

struct VertexLayout
{
	uint64 VertexDataSize = UINT64_MAX;
	void* VertexData = nullptr;
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

	 uint CreateMesh(const String& key, MeshTypeEnum meshType, VertexLayout& vertexData, VkGuid materialId = VkGuid());
	 uint CreateMesh(const String& key, MeshTypeEnum meshType, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId = VkGuid());
	 uint CreateSpriteLayer(uint32 spriteMeshId);
	 uint CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec3& color);
	 uint CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec4& color);
	 uint CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec3& startColor, const vec3& endColor);
	 uint CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec4& startColor, const vec4& endColor);
	 uint CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec3& color);
	 uint CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec4& color);
	 uint CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec3& startColor, const vec3& endColor);
	 uint CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec4& startColor, const vec4& endColor);
	 const Vector<Mesh> FindMeshByMeshKey(const String& meshKey);
	 const Vector<Mesh> FindMeshByMeshType(MeshTypeEnum meshType);
	 const Vector<MeshDrawMessage> DrawMesh(const String& meshKey);
	 const Vector<MeshDrawMessage> DrawMesh(MeshTypeEnum meshType);
	 const Vector<MeshDrawMessage> DrawInstancedMesh(uint32 instanceMeshId, Vector<SpriteLayer>& spriteLayerList);

	 void Update(const float& deltaTime);
	 void Destroy(uint meshId);
	 void DestroyAllGameObjects();
	 const Mesh& FindMesh(const uint& meshId);
	 MeshAssetData& FindMeshAssetData(const uint64& meshAssetId);
};
extern  MeshSystem& meshSystem;
inline MeshSystem& MeshSystem::Get()
{
	static MeshSystem instance;
	return instance;
}