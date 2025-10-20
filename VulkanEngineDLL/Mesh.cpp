#include "Mesh.h"
#include "BufferSystem.h"

uint NextMeshId = 0;
uint NextSpriteMeshId = 0;
uint NextLevelLayerMeshId = 0;
MeshArchive meshArchive = MeshArchive();

uint32 GetNextMeshIndex()
{
    if (!meshArchive.FreeMeshIndicesList.empty())
    {
        uint index = meshArchive.FreeMeshIndicesList.back();
        meshArchive.FreeMeshIndicesList.pop_back();
        return index;
    }
    return meshArchive.MeshList.size();
}

uint Mesh_CreateMesh(const GraphicsRenderer& renderer, MeshTypeEnum meshType, Vector<Vertex2D>& vertexList, Vector<uint32>& indexList)
{
    uint meshId = GetNextMeshIndex();
    mat4 meshMatrix = mat4(1.0f);
    MeshPropertiesStruct meshProperties = MeshPropertiesStruct();
    const VkBufferUsageFlags MeshBufferUsageSettings = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    const VkMemoryPropertyFlags MeshBufferPropertySettings = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Mesh mesh = Mesh
    {
        .MeshId = meshId,
        .ParentGameObjectId = UINT32_MAX,
        .MeshTypeId = static_cast<uint32>(meshType),
        .VertexTypeId = BufferTypeEnum::BufferType_Vertex2D,
        .MeshPropertiesId = meshId,
        .MeshVertexBufferId = bufferSystem.CreateVulkanBuffer<Vertex2D>(renderer, vertexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
        .MeshIndexBufferId = bufferSystem.CreateVulkanBuffer<uint32>(renderer, indexList, MeshBufferUsageSettings, MeshBufferPropertySettings, true),
        .MeshTransformBufferId = bufferSystem.CreateVulkanBuffer<mat4>(renderer, meshMatrix, MeshBufferUsageSettings, MeshBufferPropertySettings, false),
        .PropertiesBufferId = bufferSystem.CreateVulkanBuffer<MeshPropertiesStruct>(renderer, meshProperties, MeshBufferUsageSettings, MeshBufferPropertySettings, false),
        .VertexIndex = meshId,
        .IndexIndex = meshId,
        .MeshPosition = vec3(0.0f),
        .MeshRotation = vec3(0.0f),
        .MeshScale = vec3(1.0f),
        .MaterialId = VkGuid(),
        .MeshExtension = nullptr
    };

    meshArchive.MeshList.emplace_back(mesh);
    meshArchive.MeshPropertiesList.emplace_back(meshProperties);
    meshArchive.Vertex2DList.emplace_back(vertexList);
    meshArchive.IndexList.emplace_back(indexList);

    shaderArchive.PipelineShaderStructMap[mesh.PropertiesBufferId] = Shader_CopyShaderStructProtoType("MeshProperitiesBuffer");
    shaderArchive.PipelineShaderStructMap[mesh.PropertiesBufferId].ShaderStructBufferId = mesh.PropertiesBufferId;
    return meshId;
}

void Mesh_Update(const GraphicsRenderer& renderer, const float& deltaTime)
{
    for (auto& mesh : meshArchive.MeshList)
    {
        VulkanBuffer& propertiesBuffer = bufferSystem.VulkanBufferMap[mesh.PropertiesBufferId];
        uint32 shaderMaterialBufferIndex = (mesh.MaterialId != VkGuid()) ? Material_FindMaterial(mesh.MaterialId).ShaderMaterialBufferIndex : 0;
        Mesh_UpdateMesh(renderer, mesh, shaderArchive.PipelineShaderStructMap[mesh.PropertiesBufferId], propertiesBuffer, shaderMaterialBufferIndex, deltaTime);
    }
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

void Mesh_Destroy(const GraphicsRenderer& renderer, uint meshId)
{
    Mesh& mesh = meshArchive.MeshList[meshId];
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
    for (auto& mesh : meshArchive.MeshList)
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

const Mesh& Mesh_FindMesh(const uint& meshId)
{
    return meshArchive.MeshList[meshId];
}

const Vector<Mesh> Mesh_FindMeshByMeshType(MeshTypeEnum meshType)
{
    Vector<Mesh> meshList;
    std::copy_if(meshArchive.MeshList.begin(), meshArchive.MeshList.end(), std::back_inserter(meshList),
        [meshType](const Mesh& mesh) 
        { 
            return mesh.MeshTypeId == static_cast<uint32>(meshType);
        });
    return meshList;
}

const Vector<Mesh>& Mesh_FindMeshByVertexType(VertexTypeEnum vertexType)
{
    Vector<Mesh> meshList;
    std::copy_if(meshArchive.MeshList.begin(), meshArchive.MeshList.end(), std::back_inserter(meshList),
        [vertexType](const Mesh& mesh)
        {
            return mesh.VertexTypeId == static_cast<uint32>(vertexType);
        });
    return meshList;
}

const Vector<Mesh> Mesh_MeshList()
{
    return meshArchive.MeshList;
}