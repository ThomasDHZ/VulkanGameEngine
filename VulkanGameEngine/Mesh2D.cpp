#include "Mesh2D.h"
#include "MemoryManager.h"

Mesh2D::Mesh2D() : Mesh()
{
}

Mesh2D::Mesh2D(Vector<Vertex2D>& vertexList, Vector<uint32>& indexList, SharedPtr<Material> material) : Mesh()
{
	MeshStartUp<Vertex2D>(vertexList, indexList, material);
}

Mesh2D::~Mesh2D()
{
}

void Mesh2D::Update(const float& deltaTime)
{
	Mesh::Update(deltaTime);
}

void Mesh2D::Draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& shaderPipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties)
{
	sceneProperties.MeshBufferIndex = MeshBufferIndex;

	VkDeviceSize offsets[] = { 0 };
	vkCmdPushConstants(commandBuffer, shaderPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SceneDataBuffer), &sceneProperties);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &MeshVertexBuffer.Buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, MeshIndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, IndexCount, 1, 0, 0, 0);
}

void Mesh2D::InstanceDraw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& shaderPipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties, VulkanBuffer<SpriteInstanceStruct>& InstanceBuffer)
{
}

void Mesh2D::Destroy()
{
	Mesh::Destroy();
}
