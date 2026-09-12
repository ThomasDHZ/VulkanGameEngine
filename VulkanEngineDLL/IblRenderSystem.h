#pragma once
#include "DLL.h"
#include <Platform.h>
#include "RenderSystem.h"

class ENGINE_DLL_EXPORT IblRenderSystem
{
public:
	static IblRenderSystem& Get();

private:

	VkGuid		   _environmentMap;
	VkGuid		   _brdfRenderPassId;
	VkGuid		   _environmentToCubeMapRenderPassId;
	VkGuid		   _irradianceMapRenderPassId;
	VkGuid		   _prefilterMapRenderPassId;
	Vector<VkGuid> _renderPassDrawList;


public:
	IblRenderSystem() = default;
	~IblRenderSystem() = default;
	IblRenderSystem(const IblRenderSystem&) = delete;
	IblRenderSystem& operator=(const IblRenderSystem&) = delete;
	IblRenderSystem(IblRenderSystem&&) = delete;
	Vector<RenderPassNode> CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime);
	IblRenderSystem& operator=(IblRenderSystem&&) = delete;

	void StartUp(const String& texturePath);
	void SetEnvironmentMap(const String& texturePath);
};
ENGINE_DLL_EXPORT extern  IblRenderSystem& iblRenderSystem;
inline IblRenderSystem& IblRenderSystem::Get()
{
	static IblRenderSystem instance;
	return instance;
}

