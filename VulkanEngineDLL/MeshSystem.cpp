#include "MeshSystem.h"
#include "BufferSystem.h"
#include "MaterialSystem.h"

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
    ObjectDataPool.CreateMemoryPool(4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
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

    uint32 objectIndex = ObjectDataPool.AllocateObject();
    MeshPropertiesStruct& props = ObjectDataPool.Get(objectIndex);
    props.MeshTransform = mat4(1.0f);
    props.ShaderMaterialBufferIndex = materialId != VkGuid() ? materialSystem.FindMaterial(materialId).ShaderMaterialBufferIndex : 0;

    Mesh mesh = Mesh
    {
        .MeshId = meshId,
        .SharedMeshAssetId = meshHash,
        .ObjectDataIndex = objectIndex,
        .Type = meshType,
        .Layout = vertexData.VertexType,
        .Position = vec3(0.0f),
        .Rotation = vec3(0.0f),
        .Scale = vec3(1.0f),
        .MaterialId = materialId,
    };
    MeshList.emplace_back(mesh);
    return meshId;
}

void MeshSystem::Update(const float& deltaTime)
{
    bool anyDirty = false;
    for (auto& mesh : MeshList)
    {
        if (mesh.MeshId == UINT32_MAX) continue;

        MeshPropertiesStruct& meshProperties = ObjectDataPool.Get(mesh.ObjectDataIndex);

        mat4 model = mat4(1.0f);
        model = glm::translate(model, mesh.Position);
        model = glm::rotate(model, glm::radians(mesh.Rotation.x), vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(mesh.Rotation.y), vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(mesh.Rotation.z), vec3(0, 0, 1));
        model = glm::scale(model, mesh.Scale);
        bool changed = (meshProperties.MeshTransform != model);

        uint32 matIndex = mesh.MaterialId != VkGuid() ? materialSystem.FindMaterial(mesh.MaterialId).ShaderMaterialBufferIndex : 0;
        if (meshProperties.ShaderMaterialBufferIndex != matIndex)
        {
            meshProperties.ShaderMaterialBufferIndex = matIndex;
            changed = true;
        }

        if (changed)
        {
            meshProperties.MeshTransform = model;
            anyDirty = true;
        }
    }

    if (anyDirty)
    {
        ObjectDataPool.MarkDirty();
    }
    ObjectDataPool.UploadIfDirty();
}

void MeshSystem::Destroy(uint meshId)
{
    Mesh& mesh = MeshList[meshId];

    ObjectDataPool.FreeDataSlot(mesh.ObjectDataIndex);

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
            ObjectDataPool.FreeDataSlot(mesh.ObjectDataIndex);
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

VkDescriptorBufferInfo MeshSystem::GetMeshPropertiesBuffer()
{
        const VulkanBuffer& objBuffer = bufferSystem.FindVulkanBuffer(ObjectDataPool.BufferId);
        VkDescriptorBufferInfo objectInfo
        {
            .buffer = objBuffer.Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };
        return objectInfo;
}

uint64 MeshSystem::HashAssetKey(std::string_view key)
{
    return XXH64(key.data(), key.size(), 0);
}
