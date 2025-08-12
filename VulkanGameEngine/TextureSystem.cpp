#include "TextureSystem.h"

TextureSystem textureSystem = TextureSystem();

VkGuid TextureSystem::LoadTexture(const String& texturePath)
{
    if (texturePath.empty() ||
        texturePath == "")
    {
        return VkGuid();
    }

    nlohmann::json json = Json::ReadJson(texturePath);
    VkGuid textureId = VkGuid(json["TextureId"].get<String>().c_str());

    auto it = textureSystem.TextureMap.find(textureId);
    if (it != textureSystem.TextureMap.end())
    {
        return textureId;
    }

    TextureMap[textureId] = Texture_LoadTexture(renderSystem.renderer, texturePath.c_str());
    return textureId;
}

void TextureSystem::Update(const float& deltaTime)
{
    int x = 0;
    for (auto& [id, texture] : TextureMap)
    {
        UpdateTextureBufferIndex(texture, x);
        x++;
    }
}

Texture TextureSystem::CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo)
{
	return Texture_CreateTexture(renderSystem.renderer, textureId, imageType, createImageInfo, samplerCreateInfo);
}

Texture TextureSystem::CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
{
	return Texture_CreateTexture(renderSystem.renderer, texturePath, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
}

Texture TextureSystem::CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
{
	return Texture_CreateTexture(renderSystem.renderer, clearColor, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
}

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
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, texture.textureImageLayout, newImageLayout);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateTextureLayout(renderSystem.renderer, texture, oldImageLayout, newImageLayout);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
{
	Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    Texture_UpdateCmdTextureLayout(renderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout);
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
    auto it = TextureMap.find(guid);
    if (it != TextureMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("TextureMap not found for given GUID");
}

Texture& TextureSystem::FindDepthTexture(const RenderPassGuid& guid)
{
    auto it = DepthTextureMap.find(guid);
    if (it != DepthTextureMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("DepthTextureMap not found for given GUID");
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
        {
            return *it;
        }
    }
    throw std::out_of_range("Texture with given ID not found");
}

Vector<Texture>& TextureSystem::FindRenderedTextureList(const RenderPassGuid& guid)
{
    auto it = RenderedTextureListMap.find(guid);
    if (it != RenderedTextureListMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("RenderedTextureList not found for given GUID");
}

 bool TextureSystem::TextureExists(const RenderPassGuid& guid)
 {
     auto it = TextureMap.find(guid);
     if (it != TextureMap.end())
     {
         return true;
     }
     return false;
 }

 bool TextureSystem::DepthTextureExists(const RenderPassGuid& guid)
 {
     auto it = DepthTextureMap.find(guid);
     if (it != DepthTextureMap.end())
     {
         return true;
     }
     return false;
 }

 bool TextureSystem::RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid)
 {
     auto exists = [&](const RenderPassGuid& g, const TextureGuid& tGuid) -> bool
         {
             auto it = RenderedTextureListMap.find(g);
             if (it != RenderedTextureListMap.end())
             {
                 auto texIt = std::find_if(it->second.begin(), it->second.end(),
                     [&](const Texture& texture) { return texture.textureId == tGuid; });
                 if (texIt != it->second.end())
                 {
                     return true;
                 }
             }
             return false;
         };
     return exists(guid, textureGuid);
 }

 bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& guid)
 {
     auto it = RenderedTextureListMap.find(guid);
     if (it != RenderedTextureListMap.end())
     {
         return true;
     }
     return false;
 }

const Vector<Texture> TextureSystem::TextureList()
{
    Vector<Texture> textureList;
    for (const auto& texture : TextureMap)
    {
        textureList.emplace_back(texture.second);
    }
    return textureList;
}

const Vector<Texture> TextureSystem::DepthTextureList()
{
    Vector<Texture> depthTextureList;
    for (const auto& depthTextureMesh : DepthTextureMap)
    {
        depthTextureList.emplace_back(depthTextureMesh.second);
    }
    return depthTextureList;
}

void TextureSystem::DestroyAllTextures()
{
    for (auto& texture : TextureMap)
    {
        Texture_DestroyTexture(renderSystem.renderer, texture.second);
    }
    for (auto& texture : DepthTextureMap)
    {
        Texture_DestroyTexture(renderSystem.renderer, texture.second);
    }
    for (auto& textureList : RenderedTextureListMap)
    {
        for (auto& texture : textureList.second)
        {
            Texture_DestroyTexture(renderSystem.renderer, texture);
        }
    }

    TextureMap.clear();
    DepthTextureMap.clear();
    RenderedTextureListMap.clear();
}