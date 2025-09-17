#include "TextureSystem.h"

TextureSystem textureSystem = TextureSystem();

// Load Texture
VkGuid TextureSystem::LoadTexture(const String& texturePath)
{
    if (texturePath.empty() || texturePath == "")
        return VkGuid();

    nlohmann::json json = Json::ReadJson(texturePath);
    VkGuid textureId = VkGuid(json["TextureId"].get<String>().c_str());

    auto it = textureSystem.TextureMap.find(textureId);
    if (it != textureSystem.TextureMap.end())
        return textureId;

    TextureMap[textureId] = Texture_LoadTexture(renderSystem.renderer, texturePath.c_str());
    return textureId;
}

// Update
void TextureSystem::Update(const float& deltaTime)
{
    int x = 0;
    for (auto& [id, texture] : TextureMap)
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
    Texture_UpdateTextureSize(renderSystem.renderer, texture, imageType, TextureResolution);
}

void TextureSystem::GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
{
    Texture_GetTexturePropertiesBuffer(texture, textureDescriptorList);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout)
{
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, oldImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
{
    Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
{
    Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::DestroyTexture(Texture& texture)
{
    Texture_DestroyTexture(renderSystem.renderer, texture);
}

void TextureSystem::AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList)
{
    RenderedTextureListMap[vkGuid] = renderedTextureList;
}

void TextureSystem::AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture)
{
    DepthTextureMap[vkGuid] = depthTexture;
}

Texture& TextureSystem::FindTexture(const RenderPassGuid& guid)
{
    return TextureMap.at(guid);
}

Texture& TextureSystem::FindDepthTexture(const RenderPassGuid& guid)
{
    return DepthTextureMap.at(guid);
}

Texture& TextureSystem::FindRenderedTexture(const TextureGuid& textureGuid)
{
    for (auto& pair : RenderedTextureListMap)
    {
        auto& textureList = pair.second;
        auto it = std::find_if(textureList.begin(), textureList.end(),
            [&textureGuid](const Texture& texture)
            {
                return texture.textureId == textureGuid;
            });
        if (it != textureList.end())
            return *it;
    }
    throw std::out_of_range("Texture with given ID not found");
}

Vector<Texture>& TextureSystem::FindRenderedTextureList(const RenderPassGuid& guid)
{
    return RenderedTextureListMap.at(guid);
}

const bool TextureSystem::TextureExists(const RenderPassGuid& guid) const
{
    return TextureMap.contains(guid);
}

const bool TextureSystem::DepthTextureExists(const RenderPassGuid& guid) const
{
    return DepthTextureMap.contains(guid);
}

const bool TextureSystem::RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid) const
{
    auto it = RenderedTextureListMap.find(guid);
    if (it != RenderedTextureListMap.end())
    {
        return std::any_of(it->second.begin(), it->second.end(),
            [&textureGuid](const Texture& texture) { return texture.textureId == textureGuid; });
    }
    return RenderedTextureListMap.contains(textureGuid);
}

const bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& guid) const
{
    return RenderedTextureListMap.find(guid) != RenderedTextureListMap.end();
}

const Vector<Texture> TextureSystem::TextureList()
{
    Vector<Texture> list;
    list.reserve(TextureMap.size());
    for (const auto& pair : TextureMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<Texture> TextureSystem::DepthTextureList()
{
    Vector<Texture> list;
    list.reserve(DepthTextureMap.size());
    for (const auto& pair : DepthTextureMap)
        list.emplace_back(pair.second);
    return list;
}

void TextureSystem::DestroyAllTextures()
{
    for (auto& pair : TextureMap)
    {
        Texture_DestroyTexture(renderSystem.renderer, pair.second);
    }
    TextureMap.clear();
    
    for (auto& pair : DepthTextureMap)
    {
        Texture_DestroyTexture(renderSystem.renderer, pair.second);
    }
    DepthTextureMap.clear();
    
    for (auto& list : RenderedTextureListMap)
    {
        for (auto& texture : list.second)
        {
            Texture_DestroyTexture(renderSystem.renderer, texture);
        }
    }
    RenderedTextureListMap.clear();
}