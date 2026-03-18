#pragma once
#include <TextureSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT bool                  TextureSystem_CreateTexture(const char* texturePath);
    DLL_EXPORT bool                  TextureSystem_LoadKTXTexture(const char* texturePath);
    DLL_EXPORT bool                  TextureSystem_GenerateTexture(VkGuid renderPassId);
    DLL_EXPORT bool                  TextureSystem_GenerateCubeMapTexture(VkGuid renderPassId);
    DLL_EXPORT bool                  TextureSystem_CreateRenderPassTexture(VulkanRenderPass& vulkanRenderPass, uint attachmentId);
    DLL_EXPORT void                  TextureSystem_DestroyTexture(VkGuid textureGuid);
    DLL_EXPORT void                  TextureSystem_DestroyAllTextures();
    DLL_EXPORT const bool            TextureSystem_TextureExists(VkGuid textureGuid);
    DLL_EXPORT const bool            TextureSystem_DepthTextureExists(VkGuid renderPassGuid);
    DLL_EXPORT const bool            TextureSystem_RenderedTextureExists(VkGuid renderPassGuid, VkGuid textureGuid);
#ifdef __cplusplus
}
#endif