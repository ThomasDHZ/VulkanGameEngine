#include "TextureSystem.h"
#include "VulkanFileSystem.h"

TextureSystem textureSystem = TextureSystem();

TextureSystem::TextureSystem()
{
    textureArchivePtr = &textureArchive;
}

TextureSystem::~TextureSystem()
{
}

// Load Texture
VkGuid TextureSystem::LoadTexture(const String& texturePath)
{
    if (texturePath.empty() || 
        texturePath == "")
    {
        return VkGuid();
    }

    nlohmann::json json = vulkanFileSystem.LoadJsonFile(texturePath);
    VkGuid textureId = VkGuid(json["TextureId"].get<String>().c_str());

    if(TextureExists(textureId)) return textureId;
    textureArchivePtr->TextureMap[textureId] = Texture_LoadTexture(renderer, texturePath.c_str());
    return textureId;
}

// Update
void TextureSystem::Update(const float& deltaTime)
{
    int x = 0;
    for (auto& [id, texture] : textureArchivePtr->TextureMap)
    {
        UpdateTextureBufferIndex(texture, x);
        x++;
    }
}

// Create Texture
//Texture TextureSystem::CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
//{
//    return Texture_CreateTexture(renderSystem.renderer, textureId, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
//}
//
//Texture TextureSystem::CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
//{
//    return Texture_CreateTexture(renderSystem.renderer, texturePath, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
//}
//
//Texture TextureSystem::CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
//{
//    return Texture_CreateTexture(renderSystem.renderer, clearColor, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
//}

// Texture Update & Layouts
void TextureSystem::UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex)
{
    Texture_UpdateTextureBufferIndex(texture, bufferIndex);
}

void TextureSystem::UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution)
{
    Texture_UpdateTextureSize(renderer, texture, imageType, TextureResolution);
}

void TextureSystem::GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
{
    Texture_GetTexturePropertiesBuffer(texture, textureDescriptorList);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout)
{
    Texture_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
{
    Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::DestroyTexture(Texture& texture)
{
    Texture_DestroyTexture(renderer, texture);
}

void TextureSystem::AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList)
{
    Texture_AddRenderedTexture(vkGuid, renderedTextureList);
}

void TextureSystem::AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture)
{
    Texture_AddDepthTexture(vkGuid, depthTexture);
}

Texture& TextureSystem::FindTexture(const RenderPassGuid& guid)
{
    return Texture_FindTexture(guid);
}

Texture& TextureSystem::FindDepthTexture(const RenderPassGuid& guid)
{
    return Texture_FindDepthTexture(guid);
}

Texture& TextureSystem::FindRenderedTexture(const TextureGuid& textureGuid)
{
    return Texture_FindRenderedTexture(textureGuid);
}

Vector<Texture>& TextureSystem::FindRenderedTextureList(const RenderPassGuid& guid)
{
    return Texture_FindRenderedTextureList(guid);
}

const bool TextureSystem::TextureExists(const RenderPassGuid& guid) const
{
    return Texture_TextureExists(guid);
}

const bool TextureSystem::DepthTextureExists(const RenderPassGuid& guid) const
{
    return Texture_DepthTextureExists(guid);
}

const bool TextureSystem::RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid) const
{
    return Texture_RenderedTextureExists(guid, textureGuid);
}

const bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& guid) const
{
    return Texture_RenderedTextureListExists(guid);
}

const Vector<Texture> TextureSystem::TextureList()
{
    Vector<Texture> list;
    list.reserve(textureArchivePtr->TextureMap.size());
    for (const auto& pair : textureArchivePtr->TextureMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<Texture> TextureSystem::DepthTextureList()
{
    Vector<Texture> list;
    list.reserve(textureArchivePtr->DepthTextureMap.size());
    for (const auto& pair : textureArchivePtr->DepthTextureMap)
        list.emplace_back(pair.second);
    return list;
}

void TextureSystem::DestroyAllTextures()
{
    Texture_DestroyAllTextures();
}