//#include "RenderMesh2DComponent.h"
//#include "MemoryManager.h"
//
//RenderMesh2DComponent::RenderMesh2DComponent() : GameObjectComponent()
//{
//}
//
//RenderMesh2DComponent::RenderMesh2DComponent(SharedPtr<GameObject> parentGameObjectPtr, String name, uint32 meshBufferIndex) : GameObjectComponent(this, parentGameObjectPtr, name, ComponentTypeEnum::kRenderMesh2DComponent)
//{
//	std::vector<Vertex2D> SpriteVertexList =
//	{
//	  { {0.0f, 0.5f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
//	  { {0.5f, 0.5f},  {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
//	  { {0.5f, 0.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
//	  { {0.0f, 0.0f},  {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} }
//	};
//	std::vector<uint32> SpriteIndexList =
//	{
//	  0, 1, 3,
//	  1, 2, 3
//	};
//
//	mesh = std::make_shared<Mesh2D>(std::make_shared<RenderMesh2DComponent>(*this), SpriteVertexList, SpriteIndexList, meshBufferIndex);
//}
//
//RenderMesh2DComponent::~RenderMesh2DComponent()
//{
//}
//
//SharedPtr<RenderMesh2DComponent> RenderMesh2DComponent::CreateRenderMesh2DComponent(SharedPtr<GameObject> parentGameObjectPtr, String name, uint32 meshBufferIndex)
//{
//	std::vector<Vertex2D> SpriteVertexList =
//	{
//	  { {0.0f, 0.5f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
//	  { {0.5f, 0.5f},  {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
//	  { {0.5f, 0.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
//	  { {0.0f, 0.0f},  {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} }
//	};
//	std::vector<uint32> SpriteIndexList =
//	{
//	  0, 1, 3,
//	  1, 2, 3
//	};
//
//	SharedPtr<RenderMesh2DComponent> gameObject = MemoryManager::AllocateRenderMesh2DComponent();
//	new (gameObject.get()) RenderMesh2DComponent(parentGameObjectPtr, name, meshBufferIndex);
//	return gameObject;
//}
//
//void RenderMesh2DComponent::Input(float deltaTime)
//{
//}
//
//void RenderMesh2DComponent::Update(float deltaTime)
//{
//	mesh->Update(deltaTime);
//}
//
//void RenderMesh2DComponent::BufferUpdate(VkCommandBuffer& commandBuffer, float deltaTime)
//{
//	mesh->BufferUpdate(commandBuffer, deltaTime);
//	mesh->Update(deltaTime);
//}
//
//void RenderMesh2DComponent::Draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties)
//{
//	mesh->Draw(commandBuffer, pipeline, pipelineLayout, descriptorSet, sceneProperties);
//}
//
//void RenderMesh2DComponent::Destroy()
//{
//	mesh->Destroy();
//}
//
//SharedPtr<GameObjectComponent> RenderMesh2DComponent::Clone() const
//{
//	return std::make_shared<RenderMesh2DComponent>(*this);
//}
//
//size_t RenderMesh2DComponent::GetMemorySize() const
//{
//	return sizeof(RenderMesh2DComponent);
//}