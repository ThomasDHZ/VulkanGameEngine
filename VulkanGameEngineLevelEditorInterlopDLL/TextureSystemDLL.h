#pragma once
#include <TextureSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT VkGuid   TextureSystem_CreateTexture(const char* texturePath);
    DLL_EXPORT void     TextureSystem_AddRenderedTexture(RenderPassGuid& renderPassGuid, Texture* renderedTextureListPtr, size_t renderTextureCount);
    DLL_EXPORT void     TextureSystem_AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void     TextureSystem_Update(const float& deltaTime);
    DLL_EXPORT void     TextureSystem_UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateCmdTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& textureResolution);
    DLL_EXPORT Texture  TextureSystem_FindTexture(const RenderPassGuid& textureGuid);
    DLL_EXPORT Texture& TextureSystem_FindDepthTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture& TextureSystem_FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_TextureExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT bool     TextureSystem_DepthTextureExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureListExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT void     TextureSystem_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void     TextureSystem_DestroyTexture(Texture& texture);
    DLL_EXPORT void     TextureSystem_DestroyAllTextures();
#ifdef __cplusplus
}
#endif
DLL_EXPORT Vector<Texture>& TextureSystem_FindRenderedTextureList(const RenderPassGuid& guid);