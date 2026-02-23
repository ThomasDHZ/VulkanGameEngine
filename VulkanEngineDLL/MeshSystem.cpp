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

void MeshSystem::StartUp()
{
	//ObjectDataPool.CreateMemoryPool(4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	//constexpr size_t InitialCapacity = 4096;
	//MeshPropertiesPool.CreateMemoryPool(InitialCapacity, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
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
				.VertexBufferId = bufferSystem.VMACreateStaticVulkanBuffer(vertexData.VertexData, vertexData.VertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
				.IndexBufferId = bufferSystem.VMACreateVulkanBuffer<uint32>(indexList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true),
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

void MeshSystem::Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList)
{
	for (int x = 0; x < MeshList.size(); x++)
	{
		if (MeshList[x].MeshId == UINT32_MAX) continue;

		mat4 model = mat4(1.0f);
		model = glm::translate(model, MeshList[x].Position);
		model = glm::rotate(model, glm::radians(MeshList[x].Rotation.x), vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(MeshList[x].Rotation.y), vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(MeshList[x].Rotation.z), vec3(0, 0, 1));
		model = glm::scale(model, MeshList[x].Scale);

		MeshPropertiesStruct& meshProperties = memoryPoolSystem.UpdateMesh(MeshList[x].ObjectDataIndex);
		
		bool changed = (memoryPoolSystem.UpdateMesh(x).MeshTransform != model);
		uint32 matIndex = MeshList[x].MaterialId != VkGuid() ? materialSystem.FindMaterialPoolIndex(MeshList[x].MaterialId) : 0;
		if (meshProperties.MaterialIndex != matIndex)
		{
			meshProperties.MaterialIndex = matIndex;
			changed = true;
		}

		if (changed)
		{
			meshProperties.MeshTransform = model;
			memoryPoolSystem.MarkMemoryPoolBufferDirty();
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