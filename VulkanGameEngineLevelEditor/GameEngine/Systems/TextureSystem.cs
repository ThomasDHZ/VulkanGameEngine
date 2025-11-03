using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.Core.Native;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;


namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public struct Texture
    {
        public Guid textureId { get; set; } = Guid.Empty;
        public int width { get; set; } = 1;
        public int height { get; set; } = 1;
        public int depth { get; set; } = 1;
        public uint mipMapLevels { get; set; } = 1;
        public uint textureBufferIndex { get; set; } = 0;

        public VkImage textureImage { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDeviceMemory textureMemory { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkImageView textureView { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkSampler textureSampler { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDescriptorSet ImGuiDescriptorSet { get; set; } = VulkanCSConst.VK_NULL_HANDLE;

        public TextureUsageEnum textureUsage { get; set; } = TextureUsageEnum.kUse_Undefined;
        public TextureTypeEnum textureType { get; set; } = TextureTypeEnum.kType_UndefinedTexture;
        public VkFormat textureByteFormat { get; set; } = VkFormat.VK_FORMAT_UNDEFINED;
        public VkImageLayout textureImageLayout { get; set; } = VkImageLayout.VK_IMAGE_LAYOUT_UNDEFINED;
        public VkSampleCountFlagBits sampleCount { get; set; } = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT;
        public ColorChannelUsed colorChannels { get; set; } = ColorChannelUsed.ChannelRGBA;

        public Texture()
        {
        }
    };

    public unsafe static class TextureSystem
    {
        public static Guid LoadTexture(string texturePath)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_LoadTexture(texturePath));
        }

        public static void AddRenderedTexture(RenderPassGuid renderPassGuid, ListPtr<Texture> textureList)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_AddRenderedTexture(renderPassGuid, textureList.Ptr, textureList.Count));
        }

        public static void AddDepthTexture(RenderPassGuid renderPassGuid, Texture depthTexture)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_AddDepthTexture(renderPassGuid, depthTexture));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_Update(deltaTime));
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout newImageLayout)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateTextureLayout(texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1));
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout newImageLayout, UInt32 mipLevels)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateTextureLayout(texture, texture.textureImageLayout, newImageLayout, mipLevels));
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateTextureLayout(texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1));
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, UInt32 mipLevels)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateTextureLayout(texture, oldImageLayout, newImageLayout, mipLevels));
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout newImageLayout)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateCmdTextureLayout(commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1));
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout newImageLayout, UInt32 mipLevels)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateCmdTextureLayout(commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels));
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateCmdTextureLayout(commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1));
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, UInt32 mipLevels)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateCmdTextureLayout(commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels));
        }

        public static void UpdateTextureSize(Texture texture, VkImageAspectFlagBits imageType, vec2 TextureResolution)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_UpdateTextureSize(texture, imageType, TextureResolution));
        }

        public static Texture FindTexture(RenderPassGuid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_FindTexture(renderPassGuid));
        }

        public static Texture FindDepthTexture(RenderPassGuid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_FindDepthTexture(renderPassGuid));
        }

        public static Texture FindRenderedTexture(TextureGuid textureGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_FindRenderedTexture(textureGuid));
        }

        public static bool TextureExists(RenderPassGuid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_TextureExists(renderPassGuid));
        }

        public static bool DepthTextureExists(RenderPassGuid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_DepthTextureExists(renderPassGuid));
        }

        public static bool RenderedTextureExists(RenderPassGuid renderPassGuid, TextureGuid textureGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_RenderedTextureExists(renderPassGuid, textureGuid));
        }

        public static bool RenderedTextureListExists(RenderPassGuid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_RenderedTextureListExists(renderPassGuid));
        }

        public static void GetTexturePropertiesBuffer(Texture texture, List<VkDescriptorImageInfo> textureDescriptorList)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_GetTexturePropertiesBuffer(texture, textureDescriptorList));
        }

        public static void DestroyTexture(Texture texture)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_DestroyTexture(texture));
        }

        public static void DestroyAllTextures()
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_DestroyAllTextures());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Guid TextureSystem_LoadTexture([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string texturePath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_AddRenderedTexture(RenderPassGuid renderPassGuid, Texture* renderedTextureListPtr, size_t renderTextureCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_AddDepthTexture(RenderPassGuid renderPassGuid, Texture depthTexture);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_Update(float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, UInt32 mipmapLevel);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_UpdateCmdTextureLayout(VkCommandBuffer commandBuffer, Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, UInt32 mipmapLevel);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_UpdateTextureSize(Texture texture, VkImageAspectFlagBits imageType, vec2 TextureResolution);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Texture TextureSystem_FindTexture(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Texture TextureSystem_FindDepthTexture(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Texture TextureSystem_FindRenderedTexture(TextureGuid textureGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_TextureExists(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_DepthTextureExists(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_RenderedTextureExists(RenderPassGuid renderPassGuid, TextureGuid textureGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_RenderedTextureListExists(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_GetTexturePropertiesBuffer(Texture texture, List<VkDescriptorImageInfo> textureDescriptorList);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_DestroyTexture(Texture texture);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_DestroyAllTextures();
    }
}