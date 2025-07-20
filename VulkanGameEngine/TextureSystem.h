#pragma once
#include <Typedef.h>
#include "VkGuid.h"
#include <Texture.h>
#include "RenderSystem.h"

class TextureSystem
{
private:

	UnorderedMap<RenderPassGuid, Texture>						  DepthTextureMap;
	UnorderedMap<RenderPassGuid, Vector<Texture>>				  RenderedTextureListMap;
	UnorderedMap<UM_RenderPipelineID, Vector<TextureGuid>>		  InputTextureListMap;

public:

	UnorderedMap<RenderPassGuid, Texture>						  TextureMap;

	VkGuid  LoadTexture(const String& texturePath);
	Texture CreateTexture(VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo);
	Texture CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    Texture CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);

	void Update(const float& deltaTime);
	void UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
	void UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
	void GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
	void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout);
	void DestroyTexture(Texture& texture);

	void AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList);
	void AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture);
	void AddInputTexture(UM_RenderPipelineID& pipelineId, TextureGuid& inputTexture);

	Texture& FindTexture(const RenderPassGuid& guid);
	Texture& FindDepthTexture(const RenderPassGuid& guid);
	Texture& FindRenderedTexture(const RenderPassGuid& guid, const TextureGuid& textureGuid);
	Vector<Texture>& FindRenderedTextureList(const RenderPassGuid& guid);
	Vector<Texture>& FindInputTextureList(const RenderPassGuid& guid, const UM_RenderPipelineID& pipelineId);

	bool TextureExists(const RenderPassGuid& guid);
	bool DepthTextureExists(const RenderPassGuid& guid);
	bool RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid);
	bool RenderedTextureListExists(const RenderPassGuid& guid);
	bool InputTextureListExists(const RenderPassGuid& guid, const UM_RenderPipelineID& pipelineId);

	const Vector<Texture> TextureList();
	const Vector<Texture> DepthTextureList();
	const Vector<Texture> InputTextureList(const RenderPassGuid& guid, const UM_RenderPipelineID& renderPipelineId);

	void DestroyAllTextures();
};
extern TextureSystem textureSystem;
