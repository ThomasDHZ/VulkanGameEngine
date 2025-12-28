#include "MeshSystem.h"
#include "BufferSystem.h"
#include "MaterialSystem.h"

MeshSystem& meshSystem = MeshSystem::Get();

uint32 MeshSystem::GetNextMeshIndex()
{
    if (!meshSystem.FreeMeshIndicesList.empty())
    {
        uint index = meshSystem.FreeMeshIndicesList.back();
        meshSystem.FreeMeshIndicesList.pop_back();
        return index;
    }
    return meshSystem.MeshList.size();
}

uint MeshSystem::CreateMesh(MeshTypeEnum meshType, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = meshSystem.GetNextMeshIndex();
    mat4 meshMatrix = mat4(1.0f);
    MeshPropertiesStruct meshProperties = MeshPropertiesStruct();

    Mesh mesh = Mesh
    {
        .MeshId = meshId,
        .ParentGameObjectId = UINT32_MAX,
        .MeshShaderBufferIndex = static_cast<uint>(meshSystem.MeshList.size()),
        .MeshTypeId = static_cast<uint32>(meshType),
        .VertexTypeId = BufferTypeEnum::BufferType_Vertex2D,
        .MeshPropertiesId = meshId,
        .MeshVertexBufferId = bufferSystem.VMACreateVulkanBuffer<Vertex2D>(vertexList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true),
        .MeshIndexBufferId = bufferSystem.VMACreateVulkanBuffer<uint32>(indexList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true),
        .MeshTransformBufferId = bufferSystem.VMACreateVulkanBuffer<mat4>(meshMatrix, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false),
        .PropertiesBufferId = bufferSystem.VMACreateVulkanBuffer<MeshPropertiesStruct>(meshProperties, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false),
        .VertexIndex = meshId,
        .IndexIndex = meshId,
        .MeshPosition = vec3(0.0f),
        .MeshRotation = vec3(0.0f),
        .MeshScale = vec3(1.0f),
        .MaterialId = VkGuid(),
        .MeshExtension = nullptr
    };

    meshSystem.MeshList.emplace_back(mesh);
    meshSystem.MeshPropertiesList.emplace_back(meshProperties);
    meshSystem.Vertex2DList.emplace_back(vertexList);
    meshSystem.IndexList.emplace_back(indexList);

    shaderSystem.PipelineShaderStructMap[mesh.PropertiesBufferId] = shaderSystem.CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderSystem.PipelineShaderStructMap[mesh.PropertiesBufferId].ShaderStructBufferId = mesh.PropertiesBufferId;
    return meshId;
}

uint MeshSystem::CreateMesh(MeshTypeEnum meshType, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList, VkGuid& materialId)
{
    uint meshId = meshSystem.GetNextMeshIndex();
    mat4 meshMatrix = mat4(1.0f);
    MeshPropertiesStruct meshProperties = MeshPropertiesStruct();

    Mesh mesh = Mesh
    {
        .MeshId = meshId,
        .ParentGameObjectId = UINT32_MAX,
        .MeshShaderBufferIndex = static_cast<uint>(meshSystem.MeshList.size()),
        .MeshTypeId = static_cast<uint32>(meshType),
        .VertexTypeId = BufferTypeEnum::BufferType_Vertex2D,
        .MeshPropertiesId = meshId,
        .MeshVertexBufferId = bufferSystem.VMACreateVulkanBuffer<Vertex2D>(vertexList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true),
        .MeshIndexBufferId = bufferSystem.VMACreateVulkanBuffer<uint32>(indexList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true),
        .MeshTransformBufferId = bufferSystem.VMACreateVulkanBuffer<mat4>(meshMatrix, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false),
        .PropertiesBufferId = bufferSystem.VMACreateVulkanBuffer<MeshPropertiesStruct>(meshProperties, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false),
        .VertexIndex = meshId,
        .IndexIndex = meshId,
        .MeshPosition = vec3(0.0f),
        .MeshRotation = vec3(0.0f),
        .MeshScale = vec3(1.0f),
        .MaterialId = materialId,
        .MeshExtension = nullptr
    };

    meshSystem.MeshList.emplace_back(mesh);
    meshSystem.MeshPropertiesList.emplace_back(meshProperties);
    meshSystem.Vertex2DList.emplace_back(vertexList);
    meshSystem.IndexList.emplace_back(indexList);

    shaderSystem.PipelineShaderStructMap[mesh.PropertiesBufferId] = shaderSystem.CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderSystem.PipelineShaderStructMap[mesh.PropertiesBufferId].ShaderStructBufferId = mesh.PropertiesBufferId;
    return meshId;
}

void MeshSystem::Update(const float& deltaTime)
{
    for (auto& mesh : meshSystem.MeshList)
    {
        UpdateMesh(mesh, deltaTime);
    }
}

void MeshSystem::UpdateMesh(Mesh& mesh, const float& deltaTime)
{
    VulkanBuffer& meshPropertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];
    ShaderStructDLL& shaderStruct = shaderSystem.PipelineShaderStructMap[mesh.PropertiesBufferId];
    uint32 shaderMaterialBufferIndex = (mesh.MaterialId != VkGuid()) ? materialSystem.FindMaterial(mesh.MaterialId).ShaderMaterialBufferIndex : 0;

    const vec3 LastMeshPosition = mesh.MeshPosition;
    const vec3 LastMeshRotation = mesh.MeshRotation;
    const vec3 LastMeshScale = mesh.MeshScale;

    mat4 GameObjectMatrix = mat4(1.0);
    //SharedPtr<Transform2DComponent> transform = GameObjectTransform.lock();
    //if (transform)
    //{
    //		GameObjectMatrix = *transform->GameObjectMatrixTransform.get();
    //}

    mat4 MeshMatrix = mat4(1.0f);
    if (LastMeshPosition != mesh.MeshPosition)
    {
        MeshMatrix = glm::translate(MeshMatrix, mesh.MeshPosition);
    }
    if (LastMeshRotation != mesh.MeshRotation)
    {
        MeshMatrix = glm::rotate(MeshMatrix, glm::radians(mesh.MeshRotation.x), vec3(1.0f, 0.0f, 0.0f));
        MeshMatrix = glm::rotate(MeshMatrix, glm::radians(mesh.MeshRotation.y), vec3(0.0f, 1.0f, 0.0f));
        MeshMatrix = glm::rotate(MeshMatrix, glm::radians(mesh.MeshRotation.z), vec3(0.0f, 0.0f, 1.0f));
    }
    if (LastMeshScale != mesh.MeshScale)
    {
        MeshMatrix = glm::scale(MeshMatrix, mesh.MeshScale);
    }

    memcpy(shaderSystem.FindShaderPipelineStructVariable(shaderStruct, "MaterialIndex").Value.data(), &shaderMaterialBufferIndex, sizeof(uint));
    memcpy(shaderSystem.FindShaderPipelineStructVariable(shaderStruct, "MeshTransform").Value.data(), &GameObjectMatrix, sizeof(mat4));
    shaderSystem.UpdateShaderBuffer(shaderStruct, meshPropertiesBuffer.BufferId);
}

void MeshSystem::Destroy(uint meshId)
{
    Mesh& mesh = meshSystem.MeshList[meshId];
    VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
    VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
    VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
    VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

    bufferSystem.DestroyBuffer(vertexBuffer);
    bufferSystem.DestroyBuffer(indexBuffer);
    bufferSystem.DestroyBuffer(transformBuffer);
    bufferSystem.DestroyBuffer(propertiesBuffer);

    bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
}

void MeshSystem::DestroyAllGameObjects()
{
    for (auto& mesh : meshSystem.MeshList)
    {
        VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
        VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
        VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

        bufferSystem.DestroyBuffer(vertexBuffer);
        bufferSystem.DestroyBuffer(indexBuffer);
        bufferSystem.DestroyBuffer(transformBuffer);
        bufferSystem.DestroyBuffer(propertiesBuffer);

        bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
    }
}

const Mesh& MeshSystem::FindMesh(const uint& meshId)
{
    return meshSystem.MeshList[meshId];
}

const Vector<Mesh> MeshSystem::FindMeshByMeshType(MeshTypeEnum meshType)
{
    Vector<Mesh> meshList;
    std::copy_if(meshSystem.MeshList.begin(), meshSystem.MeshList.end(), std::back_inserter(meshList),
        [meshType](const Mesh& mesh)
        {
            return mesh.MeshTypeId == static_cast<uint32>(meshType);
        });
    return meshList;
}

const Vector<Mesh>& MeshSystem::FindMeshByVertexType(VertexTypeEnum vertexType)
{
    Vector<Mesh> meshList;
    std::copy_if(meshSystem.MeshList.begin(), meshSystem.MeshList.end(), std::back_inserter(meshList),
        [vertexType](const Mesh& mesh)
        {
            return mesh.VertexTypeId == static_cast<uint32>(vertexType);
        });
    return meshList;
}
