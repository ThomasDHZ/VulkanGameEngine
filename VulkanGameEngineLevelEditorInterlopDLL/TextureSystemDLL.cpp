//#include "TextureSystemDLL.h"
//
//VkGuid TextureSystem_CreateTexture(const char* texturePath)
//{
//	return textureSystem.CreateTexture(texturePath);
//}
//
//void TextureSystem_AddRenderedTexture(RenderPassGuid& renderPassGuid, Texture* renderedTextureListPtr, size_t renderTextureCount)
//{
//	Vector<Texture> renderedTextureList = Vector<Texture>(renderedTextureListPtr, renderedTextureListPtr + renderTextureCount);
//	return textureSystem.AddRenderedTexture(renderPassGuid, renderedTextureList);
//}
//
//void TextureSystem_AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture)
//{
//	return textureSystem.AddDepthTexture(renderPassGuid, depthTexture);
//}
//
//void TextureSystem_Update(const float& deltaTime)
//{
//	return textureSystem.Update(deltaTime);
//}
//
//void TextureSystem_UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel)
//{
//	return textureSystem.UpdateTextureLayout(texture, oldImageLayout, newImageLayout, mipmapLevel);
//}
//
//void TextureSystem_UpdateCmdTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel)
//{
//	return textureSystem.UpdateTextureLayout(texture, commandBuffer, oldImageLayout, newImageLayout, mipmapLevel);
//}
//
//void TextureSystem_UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& textureResolution)
//{
//	return textureSystem.UpdateTextureSize(texture, imageType, textureResolution);
//}
//
//Texture TextureSystem_FindTexture(const RenderPassGuid& textureGuid)
//{
//	return textureSystem.FindTexture(textureGuid);
//}
//
//Texture& TextureSystem_FindDepthTexture(const RenderPassGuid& renderPassGuid)
//{
//	return textureSystem.FindDepthTexture(renderPassGuid);
//}
//
//Texture& TextureSystem_FindRenderedTexture(const TextureGuid& textureGuid)
//{
//	return textureSystem.FindRenderedTexture(textureGuid);
//}
//
//bool TextureSystem_TextureExists(const RenderPassGuid& renderPassGuid)
//{
//	return textureSystem.TextureExists(renderPassGuid);
//}
//
//bool TextureSystem_DepthTextureExists(const RenderPassGuid& renderPassGuid)
//{
//	return textureSystem.DepthTextureExists(renderPassGuid);
//}
//
//bool TextureSystem_RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid)
//{
//	return textureSystem.RenderedTextureExists(renderPassGuid, textureGuid);
//}
//
//bool TextureSystem_RenderedTextureListExists(const RenderPassGuid& renderPassGuid)
//{
//	return textureSystem.RenderedTextureListExists(renderPassGuid);
//}
//
//void TextureSystem_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
//{
//	textureSystem.GetTexturePropertiesBuffer(texture, textureDescriptorList);
//}
//
//void TextureSystem_DestroyTexture(Texture& texture)
//{
//	textureSystem.DestroyTexture(texture);
//}
//
//void TextureSystem_DestroyAllTextures()
//{
//	textureSystem.DestroyAllTextures();
//}
//
//Vector<Texture>& TextureSystem_FindRenderedTextureList(const RenderPassGuid& guid)
//{
//	throw std::runtime_error("Not Implimented Yet");
//}
