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

public:

	UnorderedMap<RenderPassGuid, Texture>						  TextureMap;

	VkGuid  LoadTexture(const String& texturePath);
	Texture CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo);
	Texture CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    Texture CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);

	void Update(const float& deltaTime);
	void UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
	void UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
	void GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
	void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout);
	void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
	void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout);
	void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
	void DestroyTexture(Texture& texture);

	void AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList);
	void AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture);

	Texture& FindTexture(const RenderPassGuid& guid);
	Texture& FindDepthTexture(const RenderPassGuid& guid);
	Texture& FindRenderedTexture(const TextureGuid& textureGuid);
	Vector<Texture>& FindRenderedTextureList(const RenderPassGuid& guid);

	bool TextureExists(const RenderPassGuid& guid);
	bool DepthTextureExists(const RenderPassGuid& guid);
	bool RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid);
	bool RenderedTextureListExists(const RenderPassGuid& guid);

	const Vector<Texture> TextureList();
	const Vector<Texture> DepthTextureList();

	void DestroyAllTextures();
};
extern TextureSystem textureSystem;
