#pragma once
#include <Platform.h>
#include "RenderSystem.h"

class IblRenderSystem
{
public:
	static IblRenderSystem& Get();

private:

	VkGuid _environmentMap;
	VkGuid _brdfRenderPassId;
	VkGuid _environmentToCubeMapRenderPassId;
	VkGuid _irradianceMapRenderPassId;
	VkGuid _prefilterMapRenderPassId;

public:
	IblRenderSystem() = default;
	~IblRenderSystem() = default;
	IblRenderSystem(const IblRenderSystem&) = delete;
	IblRenderSystem& operator=(const IblRenderSystem&) = delete;
	IblRenderSystem(IblRenderSystem&&) = delete;
	IblRenderSystem& operator=(IblRenderSystem&&) = delete;

	void StartUp();
	Vector<RenderPassNode> CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime);
	void SetEnvironment(VkGuid environmentMapGuid);
};
extern  IblRenderSystem& iblRenderSystem;
inline IblRenderSystem& IblRenderSystem::Get()
{
	static IblRenderSystem instance;
	return instance;
}

