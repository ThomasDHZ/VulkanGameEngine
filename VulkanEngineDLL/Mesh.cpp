#include "Mesh.h"
#include "BufferSystem.h"

uint NextMeshId = 0;
uint NextSpriteMeshId = 0;
uint NextLevelLayerMeshId = 0;
MeshArchive meshArchive = MeshArchive();

uint Mesh_CreateSpriteLayerMesh(const GraphicsRenderer& renderer, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextSpriteMeshId;
    mat4 meshMatrix = mat4(1.0f);

    Mesh mesh = Mesh
    {
        .MeshId = meshId,
        .ParentGameObjectID = 0,
        .VertexCount = static_cast<uint32>(vertexList.size()),
        .IndexCount = static_cast<uint32>(indexList.size()),
        .MaterialId = VkGuid(),
        .VertexType = BufferTypeEnum::BufferType_Vertex2D,
        .MeshPosition = vec3(0.0f),
        .MeshRotation = vec3(0.0f),
        .MeshScale = vec3(1.0f),
        .MeshVertexBufferId = bufferSystem.CreateVulkanBuffer<Vertex2D>(renderer, vertexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
        .MeshIndexBufferId = bufferSystem.CreateVulkanBuffer<uint32>(renderer, indexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
        .MeshTransformBufferId = bufferSystem.CreateVulkanBuffer<mat4>(renderer, meshMatrix, MeshBufferUsageSettings, MeshBufferPropertySettings, false),
        .PropertiesBufferId = bufferSystem.CreateVulkanBuffer<MeshPropertiesStruct>(renderer, meshArchive.MeshMap[meshId].MeshProperties, MeshBufferUsageSettings, MeshBufferPropertySettings, false),
    };

    meshArchive.Vertex2DListMap[meshId] = vertexList;
    meshArchive.IndexListMap[meshId] = indexList;
    meshArchive.SpriteMeshMap[meshId] = mesh;

    shaderArchive.PipelineShaderStructMap[mesh.PropertiesBufferId] = Shader_CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderArchive.PipelineShaderStructMap[mesh.PropertiesBufferId].ShaderStructBufferId = mesh.PropertiesBufferId;
    return meshId;
}

uint Mesh_CreateLevelLayerMesh(const GraphicsRenderer& renderer, const VkGuid& levelId, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = ++NextLevelLayerMeshId;
    mat4 meshMatrix = mat4(1.0f);

    Vector<Mesh> meshList = 
    {
        Mesh
        {
            .MeshId = meshId,
            .ParentGameObjectID = 0,
            .VertexCount = static_cast<uint32>(vertexList.size()),
            .IndexCount = static_cast<uint32>(indexList.size()),
            .MaterialId = VkGuid(),
            .VertexType = BufferTypeEnum::BufferType_Vertex2D,
            .MeshPosition = vec3(0.0f),
            .MeshRotation = vec3(0.0f),
            .MeshScale = vec3(1.0f),
            .MeshVertexBufferId = bufferSystem.CreateVulkanBuffer<Vertex2D>(renderer, vertexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
            .MeshIndexBufferId = bufferSystem.CreateVulkanBuffer<uint32>(renderer, indexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
            .MeshTransformBufferId = bufferSystem.CreateVulkanBuffer<mat4>(renderer, meshMatrix, MeshBufferUsageSettings, MeshBufferPropertySettings, false),
            .PropertiesBufferId = bufferSystem.CreateVulkanBuffer<MeshPropertiesStruct>(renderer, meshArchive.MeshMap[meshId].MeshProperties, MeshBufferUsageSettings, MeshBufferPropertySettings, false)
        }
    };

    meshArchive.Vertex2DListMap[meshId] = vertexList;
    meshArchive.IndexListMap[meshId] = indexList;
    meshArchive.SpriteMeshMap[meshId] = meshList[0];
    meshArchive.LevelLayerMeshListMap[levelId] = meshList;

    shaderArchive.PipelineShaderStructMap[meshList[0].PropertiesBufferId] = Shader_CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderArchive.PipelineShaderStructMap[meshList[0].PropertiesBufferId].ShaderStructBufferId = meshList[0].PropertiesBufferId;
    return meshId;
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