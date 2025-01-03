#include "Sprite.h"

Sprite::Sprite()
{
}

Sprite::Sprite(SharedPtr<Material> material, vec2 spriteSize)
{
	//SpriteSize = spriteSize;
	//SpriteSheetPtr = std::make_shared<SpriteSheet>(SpriteSheet(material));
	//const float LeftSideUV = SpriteSheetPtr->LeftSideUV;
	//const float RightSideUV = SpriteSheetPtr->RightSideUV;
	//const float TopSideUV = SpriteSheetPtr->TopSideUV;
	//const float BottomSideUV = SpriteSheetPtr->BottomSideUV;

	//VertexList.emplace_back(Vertex2D(vec2(0.0f,         0.0f),         vec2(LeftSideUV, BottomSideUV),  Color));
	//VertexList.emplace_back(Vertex2D(vec2(SpriteSize.x, 0.0f),         vec2(RightSideUV, BottomSideUV), Color));
	//VertexList.emplace_back(Vertex2D(vec2(SpriteSize.x, SpriteSize.y), vec2(RightSideUV, TopSideUV),    Color));
	//VertexList.emplace_back(Vertex2D(vec2(0.0f,         SpriteSize.y), vec2(LeftSideUV, TopSideUV),     Color));

	VertexList =
	{
	  { {0.0f, 0.5f},  {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
	  { {0.5f, 0.5f},  {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
	  { {0.5f, 0.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
	  { {0.0f, 0.0f},  {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} }
	};

	IndexList =
	{
	  0, 1, 3,
	  1, 2, 3
	};

}

Sprite::~Sprite()
{
}

void Sprite::Input(float deltaTime)
{

}

void Sprite::Update(float deltaTime)
{
}

void Sprite::BufferUpdate(VkCommandBuffer& commandBuffer, float deltaTime)
{
}

void Sprite::Draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties)
{
}

void Sprite::Destroy()
{
}
