#include "MeshSystem.h"

uint MeshSystem::NextMeshId = 0;
uint MeshSystem::NextSpriteMeshId = 0;
uint MeshSystem::NextLevelLayerMeshId = 0;

MeshSystem meshSystem = MeshSystem();

MeshSystem::MeshSystem()
{

}

MeshSystem::~MeshSystem()
{

}

uint MeshSystem::CreateSpriteLayerMesh(Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextSpriteMeshId;
    mat4 meshMatrix = mat4(1.0f);

    Vertex2DListMap[meshId] = vertexList;
    IndexListMap[meshId] = indexList;

    MeshLoader meshLoader =
    {
        .ParentGameObjectID = 0,
        .MeshId = meshId,
        .MaterialId = VkGuid(),
        .VertexLoader = VertexLoaderStruct
        {
            .VertexType = BufferTypeEnum::BufferType_Vector2D,
            .MeshVertexBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofVertex = sizeof(Vertex2D),
            .VertexCount = static_cast<uint32>(vertexList.size()),
            .VertexData = static_cast<void*>(vertexList.data()),
        },
        .IndexLoader = IndexLoaderStruct
        {
            .MeshIndexBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofIndex = sizeof(uint),
            .IndexCount = static_cast<uint32>(indexList.size()),
            .IndexData = static_cast<void*>(indexList.data()),
        },
        .TransformLoader = TransformLoaderStruct
        {
            .MeshTransformBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofTransform = sizeof(mat4),
            .TransformData = static_cast<void*>(&meshMatrix),
        },
        .MeshPropertiesLoader = MeshPropertiesLoaderStruct
        {
            .PropertiesBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofMeshProperties = sizeof(MeshPropertiesStruct),
            .MeshPropertiesData = static_cast<void*>(&MeshMap[meshId].MeshProperties)
        }
    };

    Mesh mesh = Mesh_CreateMesh(
        renderSystem.renderer,
        meshLoader,
        bufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId]
    );

    shaderSystem.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = shaderSystem.CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderSystem.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId].ShaderStructBufferId = meshLoader.MeshPropertiesLoader.PropertiesBufferId;

    SpriteMeshMap[meshId] = mesh;
    return meshId;
}

uint MeshSystem::CreateLevelLayerMesh(const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextLevelLayerMeshId;
    mat4 meshMatrix = mat4(1.0f);

    Vertex2DListMap[meshId] = vertexList;
    IndexListMap[meshId] = indexList;

    MeshLoader meshLoader =
    {
        .ParentGameObjectID = 0,
        .MeshId = meshId,
        .MaterialId = VkGuid(),
        .VertexLoader = VertexLoaderStruct
        {
            .VertexType = BufferTypeEnum::BufferType_Vector2D,
            .MeshVertexBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofVertex = sizeof(Vertex2D),
            .VertexCount = static_cast<uint32>(vertexList.size()),
            .VertexData = static_cast<void*>(vertexList.data()),
        },
        .IndexLoader = IndexLoaderStruct
        {
            .MeshIndexBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofIndex = sizeof(uint),
            .IndexCount = static_cast<uint32>(indexList.size()),
            .IndexData = static_cast<void*>(indexList.data()),
        },
        .TransformLoader = TransformLoaderStruct
        {
            .MeshTransformBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofTransform = sizeof(mat4),
            .TransformData = static_cast<void*>(&meshMatrix),
        },
        .MeshPropertiesLoader = MeshPropertiesLoaderStruct
        {
            .PropertiesBufferId = static_cast<uint32>(++bufferSystem.NextBufferId),
            .SizeofMeshProperties = sizeof(MeshPropertiesStruct),
            .MeshPropertiesData = static_cast<void*>(&MeshMap[meshId].MeshProperties)
        }
    };

    Vector<Mesh> meshList = { Mesh_CreateMesh(
        renderSystem.renderer,
        meshLoader,
        bufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId]
    ) };

    shaderSystem.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = shaderSystem.CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderSystem.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId].ShaderStructBufferId = meshLoader.MeshPropertiesLoader.PropertiesBufferId;
    LevelLayerMeshListMap[levelId] = meshList;
    return meshId;
}

void MeshSystem::Update(const float& deltaTime)
{
    for (auto& meshPair : SpriteMeshMap)
    {
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[meshPair.second.PropertiesBufferId];
        uint32 shaderMaterialBufferIndex = (meshPair.second.MaterialId != VkGuid()) ? materialSystem.FindMaterial(meshPair.second.MaterialId).ShaderMaterialBufferIndex : 0;
        Mesh_UpdateMesh(renderSystem.renderer, meshPair.second, shaderSystem.PipelineShaderStructMap[meshPair.second.PropertiesBufferId], propertiesBuffer, shaderMaterialBufferIndex, deltaTime);
    }
}

void MeshSystem::Destroy(uint meshId)
{
    Mesh& mesh = MeshMap[meshId];
    VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
    VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
    VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
    VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

    Mesh_DestroyMesh(renderSystem.renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

    bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
}

void MeshSystem::DestroyAllGameObjects()
{
    for (auto& meshPair : MeshMap)
    {
        Mesh& mesh = meshPair.second;
        VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
        VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
        VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

        Mesh_DestroyMesh(renderSystem.renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

        bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
    }

    for (auto& meshPair : SpriteMeshMap)
    {
        Mesh& mesh = meshPair.second;
        VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
        VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
        VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

        Mesh_DestroyMesh(renderSystem.renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

        bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
    }

    for (auto& meshListPair : LevelLayerMeshListMap)
    {
        for (auto& mesh : meshListPair.second)
        {
            VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
            VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
            VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
            VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

            Mesh_DestroyMesh(renderSystem.renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

            bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
        }
    }
}

const Mesh& MeshSystem::FindMesh(const uint& id)
{
    return MeshMap.at(id);
}

const Mesh& MeshSystem::FindSpriteMesh(const uint& id)
{
    return SpriteMeshMap.at(id);
}

const Vector<Mesh>& MeshSystem::FindLevelLayerMeshList(const LevelGuid& guid)
{
    return LevelLayerMeshListMap.at(guid);
}

const Vector<Vertex2D>& MeshSystem::FindVertex2DList(const uint& id)
{
    return Vertex2DListMap.at(id);
}

const Vector<uint>& MeshSystem::FindIndexList(const uint& id)
{
    return IndexListMap.at(id);
}

const Vector<Mesh> MeshSystem::MeshList()
{
    Vector<Mesh> meshList;
    for (const auto& pair : MeshMap)
    {
        meshList.emplace_back(pair.second);
    }
    return meshList;
}

const Vector<Mesh> MeshSystem::SpriteMeshList()
{
    Vector<Mesh> spriteMeshList;
    for (const auto& pair : SpriteMeshMap)
    {
        spriteMeshList.emplace_back(pair.second);
    }
    return spriteMeshList;
}