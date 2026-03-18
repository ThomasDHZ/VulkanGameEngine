#include "TextureSystemDLL.h"

bool TextureSystem_CreateTexture(const char* texturePath)
{
	Texture texture = textureSystem.CreateTexture(texturePath);
	return texture.textureImage ? true : false;
}

bool TextureSystem_LoadKTXTexture(const char* texturePath)
{
	Texture texture = textureSystem.LoadKTXTexture(texturePath);
	return texture.textureImage ? true : false;
}

bool TextureSystem_GenerateTexture(VkGuid renderPassId)
{
	textureSystem.GenerateTexture(renderPassId);
	return  true;
}

bool TextureSystem_GenerateCubeMapTexture(VkGuid renderPassId)
{
	textureSystem.GenerateCubeMapTexture(renderPassId);
	return true;
}

bool TextureSystem_CreateRenderPassTexture(VulkanRenderPass& vulkanRenderPass, uint attachmentId)
{
	Texture texture = textureSystem.CreateRenderPassTexture(vulkanRenderPass, attachmentId);
	return texture.textureImage ? true : false;
}

 void TextureSystem_DestroyTexture(VkGuid textureGuid)
{
	 Texture texture = textureSystem.FindTexture(textureGuid);
	 textureSystem.DestroyTexture(texture);
}

 void TextureSystem_DestroyAllTextures()
{
	 textureSystem.DestroyAllTextures();
}

 const bool TextureSystem_TextureExists(VkGuid textureGuid)
 {
	 return textureSystem.TextureExists(textureGuid);
 }

 const bool TextureSystem_DepthTextureExists(VkGuid renderPassGuid)
 {
	return textureSystem.DepthTextureExists(renderPassGuid);
 }

 const bool TextureSystem_RenderedTextureExists(VkGuid renderPassGuid, VkGuid textureGuid)
 {
	return textureSystem.RenderedTextureExists(renderPassGuid, textureGuid);
 }
