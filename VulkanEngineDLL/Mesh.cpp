#include "Mesh.h"

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

	Span<ShaderVariable> shaderVariableList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount);
	memcpy(Shader_SearchShaderStructVar(shaderStruct, "MaterialIndex")->Value, &shaderMaterialBufferIndex, sizeof(uint));
	memcpy(Shader_SearchShaderStructVar(shaderStruct, "MeshTransform")->Value, &GameObjectMatrix, sizeof(mat4));
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
