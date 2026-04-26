#include "MeshSystem.h"
#include "BufferSystem.h"
#include "MaterialSystem.h"
#include "RenderSystem.h"

MeshSystem& meshSystem = MeshSystem::Get();

uint32 MeshSystem::GetNextMeshId()
{
	if (!FreeMeshIds.empty())
	{
		uint32 id = FreeMeshIds.back();
		FreeMeshIds.pop_back();
		return id;
	}
	return MeshList.size();
}

uint MeshSystem::CreateMesh(const String& key, MeshTypeEnum meshType, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId)
{
	uint meshId = meshSystem.GetNextMeshId();
	uint64 meshHash = HashAssetKey(key);
	uint assetId = UINT32_MAX;
	auto it = MeshAssetLookup.find(meshHash);
	if (it != MeshAssetLookup.end())
	{
		assetId = it->second;
	}
	if (assetId == UINT32_MAX)
	{
		MeshAssetDataList.emplace_back(MeshAssetData
			{
				.VertexBufferId = bufferSystem.CreateStaticVulkanBuffer(vertexData.VertexData, vertexData.VertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
				.IndexBufferId = bufferSystem.CreateVulkanBuffer<uint32>(indexList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true),
				.IndexCount = static_cast<uint>(indexList.size()),
				.Layout = vertexData.VertexType
			});
		MeshAssetLookup[meshHash] = MeshAssetDataList.size() - 1;
	}

	uint32 objectIndex = memoryPoolSystem.AllocateObject(kMeshBuffer);
	MeshPropertiesStruct& properties = memoryPoolSystem.UpdateMesh(objectIndex);
	properties.MaterialIndex = UINT32_MAX;
	properties.MeshTransform = mat4(1.0f);

	MeshList.emplace_back(Mesh
		{
			.MeshId = meshId,
			.SharedAssetId = meshHash,
			.ObjectDataIndex = static_cast<uint32>(objectIndex),
			.Type = meshType,
			.Layout = vertexData.VertexType,
			.Position = vec3(0.0f),
			.Rotation = vec3(0.0f),
			.Scale = vec3(1.0f),
			.MaterialId = materialId,
		});
	return meshId;
}

uint MeshSystem::CreateSpriteLayer(uint32 spriteMeshId)
{
	if (spriteMeshId != UINT32_MAX)
	{
		Vector<Vertex2DLayout> spriteVertexList =
		{
			Vertex2DLayout(vec2(0.0f, 1.0f), vec2(0.0f, 0.0f)),
			Vertex2DLayout(vec2(1.0f, 1.0f), vec2(1.0f, 0.0f)),
			Vertex2DLayout(vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)),
			Vertex2DLayout(vec2(0.0f, 0.0f), vec2(0.0f, 1.0f))
		};

		Vector<uint32> spriteIndexList =
		{
			0, 3, 1,
			1, 3, 2
		};

		VertexLayout vertexData =
		{
			.VertexType = VertexLayoutEnum::kVertexLayout_SpriteInstanceVertex,
			.VertexDataSize = sizeof(Vertex2DLayout) * spriteVertexList.size(),
			.VertexData = spriteVertexList.data(),
		};

		return meshSystem.CreateMesh("__SpriteMesh__", kMesh_SpriteMesh, vertexData, spriteIndexList);
	}
	return UINT32_MAX;
}

uint MeshSystem::CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec3& color)
{
	Vector<LineVertex2DLayout> lineVertexList =
	{
		{startPoint, vec4(color.x, color.y, color.z, 1.0f)},
		{endPoint  , vec4(color.x, color.y, color.z, 1.0f)}
	};
	
	Vector<uint32> lineIndexList = 
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex2DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh2D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec4& color)
{
	Vector<LineVertex2DLayout> lineVertexList =
	{
		{startPoint, color},
		{endPoint  , color}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex2DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh2D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec3& startColor, const vec3& endColor)
{
	Vector<LineVertex2DLayout> lineVertexList =
	{
		{startPoint, vec4(startColor.x, startColor.y, startColor.z, 1.0f)},
		{endPoint  , vec4(endColor.x, endColor.y, endColor.z, 1.0f)}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex2DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh2D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh2D(const vec2& startPoint, const vec2& endPoint, const vec4& startColor, const vec4& endColor)
{
	Vector<LineVertex2DLayout> lineVertexList =
	{
		{startPoint, startColor},
		{endPoint  , endColor}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex2DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh2D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec3& color)
{
	Vector<LineVertex3DLayout> lineVertexList =
	{
		{startPoint, vec4(color.x, color.y, color.z, 1.0f)},
		{endPoint  , vec4(color.x, color.y, color.z, 1.0f)}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex3DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh3D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec4& color)
{
	Vector<LineVertex3DLayout> lineVertexList =
	{
		{startPoint, color},
		{endPoint  , color}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex3DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh3D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec3& startColor, const vec3& endColor)
{
	Vector<LineVertex3DLayout> lineVertexList =
	{
		{startPoint, vec4(startColor.x, startColor.y, startColor.z, 1.0f)},
		{endPoint  , vec4(endColor.x, endColor.y, endColor.z, 1.0f)}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex3DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh3D__", kMesh_LineMesh, vertexData, lineIndexList);
}

uint MeshSystem::CreateLineMesh3D(const vec3& startPoint, const vec3& endPoint, const vec4& startColor, const vec4& endColor)
{
	Vector<LineVertex3DLayout> lineVertexList =
	{
		{startPoint, startColor},
		{endPoint  , endColor}
	};

	Vector<uint32> lineIndexList =
	{
		0, 1
	};

	VertexLayout vertexData =
	{
		.VertexType = VertexLayoutEnum::kVertexLayout_LineVertex,
		.VertexDataSize = sizeof(LineVertex3DLayout) * lineVertexList.size(),
		.VertexData = lineVertexList.data(),
	};

	return CreateMesh("__LineMesh3D__", kMesh_LineMesh, vertexData, lineIndexList);
}

void MeshSystem::Update(const float& deltaTime)
{
	for (size_t x = 0; x < MeshList.size(); ++x)
	{
		Mesh& mesh = MeshList[x];
		if (mesh.MeshId == UINT32_MAX) continue;
		if (!mesh.IsTransformDirty && !mesh.IsMaterialDirty) continue;

		bool changed = false;
		MeshPropertiesStruct& props = memoryPoolSystem.UpdateMesh(mesh.ObjectDataIndex);
		if (mesh.IsTransformDirty)
		{
			mat4 model = glm::translate(mat4(1.0f), mesh.Position);
			model = glm::rotate(model, glm::radians(mesh.Rotation.x), vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(mesh.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(mesh.Rotation.z), vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, mesh.Scale);

			if (props.MeshTransform != model)
			{
				props.MeshTransform = model;
				changed = true;
			}
			mesh.IsTransformDirty = false;
		}

		if (mesh.IsMaterialDirty)
		{
			uint32 matIndex = (mesh.MaterialId != VkGuid()) ? materialSystem.FindMaterialPoolIndex(mesh.MaterialId) : 0u;
			if (props.MaterialIndex != matIndex)
			{
				props.MaterialIndex = matIndex;
				changed = true;
			}
			mesh.IsMaterialDirty = false;
		}
	}
}

void MeshSystem::Destroy(uint meshId)
{
	Mesh& mesh = MeshList[meshId];
	//ObjectDataPool.FreeDataSlot(mesh.ObjectDataIndex);
	// Assets are shared — only destroy if no references left (optional refcounting later)
	// For now: never destroy shared assets (leak ok for simple engine, or add refcount)
	mesh.MeshId = UINT32_MAX;
	FreeMeshIds.push_back(meshId);
}

void MeshSystem::DestroyAllGameObjects()
{
	for (auto& mesh : MeshList)
	{
		if (mesh.MeshId != UINT32_MAX)
		{
			//ObjectDataPool.FreeDataSlot(mesh.ObjectDataIndex);
		}
	}
	for (auto& asset : MeshAssetDataList)
	{
		bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(asset.VertexBufferId));
		bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(asset.IndexBufferId));
	}
	MeshAssetDataList.clear();
	MeshList.clear();
	FreeMeshIds.clear();
}

const Mesh& MeshSystem::FindMesh(const uint& meshId)
{
	return MeshList[meshId];
}

const Vector<Mesh> MeshSystem::FindMeshByMeshType(MeshTypeEnum meshType)
{
	Vector<Mesh> result;
	for (const auto& mesh : MeshList)
	{
		if (mesh.Type == meshType && mesh.MeshId != UINT32_MAX)
		{
			result.emplace_back(mesh);
		}
	}
	return result;
}

MeshAssetData& MeshSystem::FindMeshAssetData(const uint64& meshAssetId)
{
	uint32 listId = MeshAssetLookup.find(meshAssetId)->second;
	return MeshAssetDataList[listId];
}

uint64 MeshSystem::HashAssetKey(std::string_view key)
{
	return XXH64(key.data(), key.size(), 0);
}