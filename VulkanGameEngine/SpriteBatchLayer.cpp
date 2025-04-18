#include "SpriteBatchLayer.h"
#include "MemoryManager.h"

SpriteBatchLayer::SpriteBatchLayer()
{

}

SpriteBatchLayer::SpriteBatchLayer(Vector<SharedPtr<GameObject>>& gameObjectList, JsonPipeline spriteRenderPipeline)
{
	SpriteRenderPipeline = spriteRenderPipeline;
	SpriteLayerMesh = std::make_shared<Mesh2D>(Mesh2D(SpriteVertexList, SpriteIndexList, nullptr));

	for (auto& gameObject : gameObjectList)
	{
		SharedPtr<Sprite> sprite = assetManager.SpriteList[gameObject->GameObjectId];
		SpriteList.emplace_back(sprite);
		SpriteInstanceList.emplace_back(sprite->SpriteInstance);
	}

	SpriteBuffer = SpriteInstanceBuffer(SpriteInstanceList, SpriteLayerMesh->GetMeshBufferUsageSettings(), SpriteLayerMesh->GetMeshBufferPropertySettings(), false);
	SortSpritesByLayer(SpriteList);
}

SpriteBatchLayer::~SpriteBatchLayer()
{
}

void SpriteBatchLayer::AddSprite(SharedPtr<Sprite> sprite)
{
	SpriteList.emplace_back(sprite);
	SortSpritesByLayer(SpriteList);
}

void SpriteBatchLayer::RemoveSprite(SharedPtr<Sprite> sprite)
{
	sprite->Destroy();
	SpriteList.erase(std::remove_if(SpriteList.begin(), SpriteList.end(),
		[&sprite](const SharedPtr<Sprite>& spritePtr) {
			return spritePtr.get() == sprite.get();
		}),
		SpriteList.end());
}

void SpriteBatchLayer::Update(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
	SpriteInstanceList.clear();
	SpriteInstanceList.reserve(SpriteList.size());
	for (auto& spritePtr : SpriteList)
	{
		spritePtr->Update(commandBuffer, deltaTime);
		SpriteInstanceList.emplace_back(spritePtr->SpriteInstance);
	}

	if (SpriteList.size())
	{
		SpriteBuffer.UpdateBufferMemory(SpriteInstanceList);
	}
}

void SpriteBatchLayer::Draw(VkCommandBuffer& commandBuffer, SceneDataBuffer& sceneDataBuffer)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdPushConstants(commandBuffer, SpriteRenderPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SceneDataBuffer), &sceneDataBuffer);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, SpriteRenderPipeline.Pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, SpriteRenderPipeline.PipelineLayout, 0, SpriteRenderPipeline.DescriptorSetList.size(), SpriteRenderPipeline.DescriptorSetList.data(), 0, nullptr);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, SpriteLayerMesh->GetVertexBuffer().get(), offsets);
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &SpriteBuffer.Buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, *SpriteLayerMesh->GetIndexBuffer().get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, SpriteIndexList.size(), SpriteInstanceList.size(), 0, 0, 0);
}

void SpriteBatchLayer::Destroy()
{
	SpriteLayerMesh->Destroy();
}

void SpriteBatchLayer::SortSpritesByLayer(std::vector<SharedPtr<Sprite>>& sprites)
{
	//std::sort(sprites.begin(), sprites.end(), [](const SharedPtr<Sprite>& spriteA, const SharedPtr<Sprite>& spriteB) {
	//		if (spriteA &&
	//			spriteB)
	//		{
	//			return spriteA->SpriteLayer > spriteB->SpriteLayer;
	//		}
	//		return true;
	//	});
}
