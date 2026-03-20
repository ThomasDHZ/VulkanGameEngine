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


namespace VulkanGameEngineLevelEditor.GameEngine
{
    public struct Texture
    {
        public Guid textureId { get; set; } = Guid.Empty;
        public int width { get; set; } = Int32.MaxValue;
        public int height { get; set; } = Int32.MaxValue;
        public int depth { get; set; } = Int32.MaxValue;
        public uint mipMapLevels { get; set; } = Int32.MaxValue;
        public uint textureBufferIndex { get; set; } = Int32.MaxValue;

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
        public static void CreateTexture(string texturePath)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_CreateTexture(texturePath));
        }

        public static void LoadKTXTexture(string texturePath)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_LoadKTXTexture(texturePath));
        }

        public static void GenerateTexture(Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_GenerateTexture(renderPassId));
        }

        public static void GenerateCubeMapTexture(Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_GenerateCubeMapTexture(renderPassId));
        }

        public static void CreateRenderPassTexture(VulkanRenderPass vulkanRenderPass, uint attachmentId)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_CreateRenderPassTexture(vulkanRenderPass, attachmentId));
        }

        public static void DestroyTexture(Guid textureGuid)
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_DestroyTexture(textureGuid));
        }

        public static void DestroyAllTextures()
        {
            DLLSystem.CallDLLFunc(() => TextureSystem_DestroyAllTextures());
        }

        public static bool TextureExists(Guid textureGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_TextureExists(textureGuid));
        }

        public static bool DepthTextureExists(Guid renderPassGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_DepthTextureExists(renderPassGuid));
        }

        public static bool RenderedTextureExists(Guid renderPassGuid, Guid textureGuid)
        {
            return DLLSystem.CallDLLFunc(() => TextureSystem_RenderedTextureExists(renderPassGuid, textureGuid));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_CreateTexture([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string texturePath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_LoadKTXTexture([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string texturePath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_GenerateTexture(Guid renderPassId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_GenerateCubeMapTexture(Guid renderPassId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_CreateRenderPassTexture(VulkanRenderPass vulkanRenderPass, uint attachmentId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_DestroyTexture(Guid textureGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void TextureSystem_DestroyAllTextures();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_TextureExists(Guid textureGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_DepthTextureExists(Guid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool TextureSystem_RenderedTextureExists(Guid renderPassGuid, Guid textureGuid);
    }
}