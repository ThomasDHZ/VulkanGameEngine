#include "Mesh.h"
#include "BufferSystem.h"

uint NextMeshId = 0;
uint NextSpriteMeshId = 0;
uint NextLevelLayerMeshId = 0;
MeshArchive meshArchive = MeshArchive();
Mesh Mesh_CreateMesh(const GraphicsRenderer& renderer, const MeshLoader& meshLoader, VulkanBuffer& outVertexBuffer, VulkanBuffer& outIndexBuffer, VulkanBuffer& outTransformBuffer, VulkanBuffer& outMeshPropertiesBuffer)
{
	Mesh mesh;
	mesh.MeshId = meshLoader.MeshId;
	mesh.ParentGameObjectID = meshLoader.ParentGameObjectID;
	mesh.VertexCount = meshLoader.VertexLoader.VertexCount;
	mesh.IndexCount = meshLoader.IndexLoader.IndexCount;
	mesh.MaterialId = meshLoader.MaterialId;
	mesh.VertexType = meshLoader.VertexLoader.VertexType;
	mesh.MeshPosition = vec3(0.0f);
	mesh.MeshRotation = vec3(0.0f);
	mesh.MeshScale = vec3(1.0f);
	mesh.MeshVertexBufferId = Mesh_CreateVertexBuffer(renderer, meshLoader.VertexLoader, outVertexBuffer);
	mesh.MeshIndexBufferId = Mesh_CreateIndexBuffer(renderer, meshLoader.IndexLoader, outIndexBuffer);
	mesh.MeshTransformBufferId = Mesh_CreateTransformBuffer(renderer, meshLoader.TransformLoader, outTransformBuffer);
	mesh.PropertiesBufferId = Mesh_CreateMeshPropertiesBuffer(renderer, meshLoader.MeshPropertiesLoader, outMeshPropertiesBuffer);
	return mesh;
}

void Mesh_UpdateMesh(const GraphicsRenderer& renderer, Mesh& mesh, ShaderStruct& shaderStruct, VulkanBuffer& meshPropertiesBuffer, uint shaderMaterialBufferIndex, const float& deltaTime)
{
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

	memcpy(Shader_SearchShaderStructVar(&shaderStruct, "MaterialIndex")->Value, &shaderMaterialBufferIndex, sizeof(uint));
	memcpy(Shader_SearchShaderStructVar(&shaderStruct, "MeshTransform")->Value, &GameObjectMatrix, sizeof(mat4));
	Shader_UpdateShaderBuffer(renderer, meshPropertiesBuffer, &shaderStruct, 1);
}

void Mesh_DestroyMesh(const GraphicsRenderer& renderer, Mesh& mesh, VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, VulkanBuffer& transformBuffer, VulkanBuffer& propertiesBuffer)
{
	VulkanBuffer_DestroyBuffer(renderer, vertexBuffer);
	VulkanBuffer_DestroyBuffer(renderer, indexBuffer);
	VulkanBuffer_DestroyBuffer(renderer, transformBuffer);
	VulkanBuffer_DestroyBuffer(renderer, propertiesBuffer);
}

int Mesh_CreateVertexBuffer(const GraphicsRenderer& renderer, const VertexLoaderStruct& vertexLoader, VulkanBuffer& outVertexBuffer)
{
	outVertexBuffer = VulkanBuffer_CreateVulkanBuffer2(renderer, vertexLoader.MeshVertexBufferId, vertexLoader.VertexData, vertexLoader.SizeofVertex, vertexLoader.VertexCount, vertexLoader.VertexType, MeshBufferUsageSettings, MeshBufferPropertySettings, true);
	return outVertexBuffer.BufferId;
}

int Mesh_CreateIndexBuffer(const GraphicsRenderer& renderer, const IndexLoaderStruct& indexLoader, VulkanBuffer& outIndexBuffer)
{
	outIndexBuffer = VulkanBuffer_CreateVulkanBuffer2(renderer, indexLoader.MeshIndexBufferId, indexLoader.IndexData, indexLoader.SizeofIndex, indexLoader.IndexCount, BufferType_UInt, MeshBufferUsageSettings, MeshBufferPropertySettings, true);
	return outIndexBuffer.BufferId;
}

int Mesh_CreateTransformBuffer(const GraphicsRenderer& renderer, const TransformLoaderStruct& transformLoader, VulkanBuffer& outTransformBuffer)
{
	outTransformBuffer = VulkanBuffer_CreateVulkanBuffer2(renderer, transformLoader.MeshTransformBufferId, transformLoader.TransformData, transformLoader.SizeofTransform, 1, BufferType_Mat4, MeshBufferUsageSettings, MeshBufferPropertySettings, false);
	return outTransformBuffer.BufferId;
}

int Mesh_CreateMeshPropertiesBuffer(const GraphicsRenderer& renderer, const MeshPropertiesLoaderStruct& meshPropertiesLoader, VulkanBuffer& outMeshPropertiesBuffer)
{
	outMeshPropertiesBuffer = VulkanBuffer_CreateVulkanBuffer2(renderer, meshPropertiesLoader.PropertiesBufferId, meshPropertiesLoader.MeshPropertiesData, meshPropertiesLoader.SizeofMeshProperties, 1, BufferType_MeshPropertiesStruct, MeshBufferUsageSettings, MeshBufferPropertySettings, false);
	return outMeshPropertiesBuffer.BufferId;
}

uint Mesh_CreateSpriteLayerMesh(const GraphicsRenderer& renderer, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextSpriteMeshId;
    mat4 meshMatrix = mat4(1.0f);

    meshArchive.Vertex2DListMap[meshId] = vertexList;
    meshArchive.IndexListMap[meshId] = indexList;

    MeshLoader meshLoader =
    {
        .ParentGameObjectID = 0,
        .MeshId = meshId,
        .MaterialId = VkGuid(),
        .VertexLoader = VertexLoaderStruct
        {
            .VertexType = BufferTypeEnum::BufferType_Vector2D,
            .MeshVertexBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofVertex = sizeof(Vertex2D),
            .VertexCount = static_cast<uint32>(vertexList.size()),
            .VertexData = static_cast<void*>(vertexList.data()),
        },
        .IndexLoader = IndexLoaderStruct
        {
            .MeshIndexBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofIndex = sizeof(uint),
            .IndexCount = static_cast<uint32>(indexList.size()),
            .IndexData = static_cast<void*>(indexList.data()),
        },
        .TransformLoader = TransformLoaderStruct
        {
            .MeshTransformBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofTransform = sizeof(mat4),
            .TransformData = static_cast<void*>(&meshMatrix),
        },
        .MeshPropertiesLoader = MeshPropertiesLoaderStruct
        {
            .PropertiesBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofMeshProperties = sizeof(MeshPropertiesStruct),
            .MeshPropertiesData = static_cast<void*>(&meshArchive.MeshMap[meshId].MeshProperties)
        }
    };

    Mesh mesh = Mesh_CreateMesh(
        renderer,
        meshLoader,
        bufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId]
    );

    shaderArchive.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = Shader_CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderArchive.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId].ShaderStructBufferId = meshLoader.MeshPropertiesLoader.PropertiesBufferId;

    meshArchive.SpriteMeshMap[meshId] = mesh;
    return meshId;
}

uint Mesh_CreateLevelLayerMesh(const GraphicsRenderer& renderer, const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextLevelLayerMeshId;
    mat4 meshMatrix = mat4(1.0f);

    meshArchive.Vertex2DListMap[meshId] = vertexList;
    meshArchive.IndexListMap[meshId] = indexList;

    MeshLoader meshLoader =
    {
        .ParentGameObjectID = 0,
        .MeshId = meshId,
        .MaterialId = VkGuid(),
        .VertexLoader = VertexLoaderStruct
        {
            .VertexType = BufferTypeEnum::BufferType_Vector2D,
            .MeshVertexBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofVertex = sizeof(Vertex2D),
            .VertexCount = static_cast<uint32>(vertexList.size()),
            .VertexData = static_cast<void*>(vertexList.data()),
        },
        .IndexLoader = IndexLoaderStruct
        {
            .MeshIndexBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofIndex = sizeof(uint),
            .IndexCount = static_cast<uint32>(indexList.size()),
            .IndexData = static_cast<void*>(indexList.data()),
        },
        .TransformLoader = TransformLoaderStruct
        {
            .MeshTransformBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofTransform = sizeof(mat4),
            .TransformData = static_cast<void*>(&meshMatrix),
        },
        .MeshPropertiesLoader = MeshPropertiesLoaderStruct
        {
            .PropertiesBufferId = static_cast<uint32>(++NextBufferId),
            .SizeofMeshProperties = sizeof(MeshPropertiesStruct),
            .MeshPropertiesData = static_cast<void*>(&meshArchive.MeshMap[meshId].MeshProperties)
        }
    };

    Vector<Mesh> meshList = { Mesh_CreateMesh(
        renderer,
        meshLoader,
        bufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId],
        bufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId]
    ) };

    shaderArchive.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = Shader_CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderArchive.PipelineShaderStructMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId].ShaderStructBufferId = meshLoader.MeshPropertiesLoader.PropertiesBufferId;
    meshArchive.LevelLayerMeshListMap[levelId] = meshList;
    return meshId;
}

void Mesh_Update(const GraphicsRenderer& renderer, const float& deltaTime)
{
    for (auto& meshPair : meshArchive.SpriteMeshMap)
    {
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[meshPair.second.PropertiesBufferId];
        uint32 shaderMaterialBufferIndex = (meshPair.second.MaterialId != VkGuid()) ? Material_FindMaterial(meshPair.second.MaterialId).ShaderMaterialBufferIndex : 0;
        Mesh_UpdateMesh(renderer, meshPair.second, shaderArchive.PipelineShaderStructMap[meshPair.second.PropertiesBufferId], propertiesBuffer, shaderMaterialBufferIndex, deltaTime);
    }
}

void Mesh_Destroy(const GraphicsRenderer& renderer, uint meshId)
{
    Mesh& mesh = meshArchive.MeshMap[meshId];
    VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
    VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
    VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
    VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

    Mesh_DestroyMesh(renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

    bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
    bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
}

void Mesh_DestroyAllGameObjects(const GraphicsRenderer& renderer)
{
    for (auto& meshPair : meshArchive.MeshMap)
    {
        Mesh& mesh = meshPair.second;
        VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
        VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
        VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

        Mesh_DestroyMesh(renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

        bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
    }

    for (auto& meshPair : meshArchive.SpriteMeshMap)
    {
        Mesh& mesh = meshPair.second;
        VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
        VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
        VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

        Mesh_DestroyMesh(renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

        bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
        bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
    }

    for (auto& meshListPair : meshArchive.LevelLayerMeshListMap)
    {
        for (auto& mesh : meshListPair.second)
        {
            VulkanBuffer& vertexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshVertexBufferId];
            VulkanBuffer& indexBuffer = bufferSystem.VulkanBufferMap[mesh.MeshIndexBufferId];
            VulkanBuffer& transformBuffer = bufferSystem.VulkanBufferMap[mesh.MeshTransformBufferId];
            VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];

            Mesh_DestroyMesh(renderer, mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer);

            bufferSystem.VulkanBufferMap.erase(mesh.MeshVertexBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.MeshIndexBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.MeshTransformBufferId);
            bufferSystem.VulkanBufferMap.erase(mesh.PropertiesBufferId);
        }
    }
}

const Mesh& Mesh_FindMesh(const uint& id)
{
    return meshArchive.MeshMap.at(id);
}

const Mesh& Mesh_FindSpriteMesh(const uint& id)
{
    return meshArchive.SpriteMeshMap.at(id);
}

const Vector<Mesh>& Mesh_FindLevelLayerMeshList(const LevelGuid& guid)
{
    return meshArchive.LevelLayerMeshListMap.at(guid);
}

const Vector<Vertex2D>& Mesh_FindVertex2DList(const uint& id)
{
    return meshArchive.Vertex2DListMap.at(id);
}

const Vector<uint>& Mesh_FindIndexList(const uint& id)
{
    return meshArchive.IndexListMap.at(id);
}

const Vector<Mesh> Mesh_MeshList()
{
    Vector<Mesh> meshList;
    for (const auto& pair : meshArchive.MeshMap)
    {
        meshList.emplace_back(pair.second);
    }
    return meshList;
}

const Vector<Mesh> Mesh_SpriteMeshList()
{
    Vector<Mesh> spriteMeshList;
    for (const auto& pair : meshArchive.SpriteMeshMap)
    {
        spriteMeshList.emplace_back(pair.second);
    }
    return spriteMeshList;
}