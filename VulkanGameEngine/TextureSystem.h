#pragma once
#include <Typedef.h>
#include "VkGuid.h"
#include <Texture.h>
#include "RenderSystem.h"

class TextureSystem
{
private:
    TextureArchive* textureArchivePtr;
public:
    TextureSystem();
    ~TextureSystem();

    VkGuid LoadTexture(const String& texturePath);
    //Texture CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    //Texture CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    //Texture CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);

    void Update(const float& deltaTime);
    void UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
    void UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
    void GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout);
    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels);
    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels);
    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout);
    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels);
    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels);
    void DestroyTexture(Texture& texture);

    void AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList);
    void AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture);

    Texture& FindTexture(const RenderPassGuid& guid);
    Texture& FindDepthTexture(const RenderPassGuid& guid);
    Texture& FindRenderedTexture(const TextureGuid& textureGuid);
    Vector<Texture>& FindRenderedTextureList(const RenderPassGuid& guid);

    const bool TextureExists(const RenderPassGuid& guid) const;
    const bool DepthTextureExists(const RenderPassGuid& guid) const;
    const bool RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid) const;
    const bool RenderedTextureListExists(const RenderPassGuid& guid) const;

    const Vector<Texture> TextureList();
    const Vector<Texture> DepthTextureList();
    void DestroyAllTextures();
};

extern TextureSystem textureSystem;