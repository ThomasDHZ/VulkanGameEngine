#include "Mesh2D.h"

Mesh2D::Mesh2D() : Mesh()
{
}

Mesh2D::Mesh2D(std::shared_ptr<GameObjectComponent> parentGameObjectComponent, List<Vertex2D>& vertexList, List<uint32>& indexList, uint32 MeshBufferIndex) : Mesh(parentGameObjectComponent)
{
	std::vector<Vertex2D> SpriteVertexList =
	{
	  { {0.5f,   0.5f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
	  { {0.5f,  -0.5f},  {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
	  { {-0.5f, -0.5f},  {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
	  { {-0.5f,  0.5f},  {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} }
	};
	std::vector<uint32> SpriteIndexList = 
	{
	  0, 3, 1,
	  1, 3, 2
	};

	MeshStartUp<Vertex2D>(SpriteVertexList, SpriteIndexList, MeshBufferIndex);
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

void Mesh2D::Destroy()
{
	Mesh::Destroy();
}
