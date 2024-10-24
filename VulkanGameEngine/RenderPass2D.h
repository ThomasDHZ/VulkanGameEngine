#pragma once
#include <vulkan/vulkan.h>
#include "RenderedColorTexture.h"
#include "Mesh2D.h"
#include "RenderPass.h"
#include "vertex.h"
#include "SceneDataBuffer.h"
#include "RenderedTexture.h"
#include "GameObject.h"

class RenderPass2D : public Renderpass
{
private:
	std::shared_ptr<RenderedTexture> renderedTexture;

public:
	RenderPass2D();
	virtual ~RenderPass2D();

	void BuildRenderPass();
	void BuildRenderPipeline();
	void UpdateRenderPass();
	VkCommandBuffer Draw(List<std::shared_ptr<GameObject>> meshList, SceneDataBuffer& sceneProperties);
	void Destroy() override;

	std::shared_ptr<RenderedTexture> GetRenderedTexture() { return renderedTexture; }
};