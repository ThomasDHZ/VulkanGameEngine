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
    public unsafe static class TextureSystem
    {
     
        public static Dictionary<Guid, Texture> TextureList { get; set; } = new Dictionary<Guid, Texture>();
        public static Dictionary<Guid, Texture> DepthTextureList { get; set; } = new Dictionary<Guid, Texture>();
        public static Dictionary<Guid, ListPtr<Texture>> RenderedTextureListMap { get; set; } = new Dictionary<Guid, ListPtr<Texture>>();

        public static Guid LoadTexture(string texturePath)
        {
            Guid guid = Texture_LoadTexture(RenderSystem.renderer, texturePath);
            return guid;
        }

        public static void Update(float deltaTime)
        {
            uint x = 0;
            foreach (var texture in TextureList)
            {
                Texture_UpdateTextureBufferIndex(texture.Value, x);
                x++;
            }
        }

        public static void GetTexturePropertiesBuffer(Texture texture, ref ListPtr<VkDescriptorImageInfo> textureDescriptorList)
        {
            VkDescriptorImageInfo textureDescriptor = new VkDescriptorImageInfo
            {
                sampler = texture.textureSampler,
                imageView = texture.textureView,
                imageLayout = VkImageLayout.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
            textureDescriptorList.Add(textureDescriptor);
        }

        public static void AddRenderedTexture(Guid vkGuid, ListPtr<Texture> renderedTextureList)
        {
            RenderedTextureListMap[vkGuid] = renderedTextureList;
        }

        public static void AddDepthTexture(Guid vkGuid, Texture depthTexture)
        {
            DepthTextureList[vkGuid] = depthTexture;
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout newImageLayout)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout newImageLayout, uint mipLevels)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint mipLevels)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, oldImageLayout, newImageLayout, mipLevels);
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout newImageLayout)
        {
            Texture_UpdateCmdTextureLayout(RenderSystem.renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout newImageLayout, uint mipLevels) 
        {
            Texture_UpdateCmdTextureLayout(RenderSystem.renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            Texture_UpdateCmdTextureLayout(RenderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint mipLevels)
        {
            Texture_UpdateCmdTextureLayout(RenderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels);
        }

        public static bool TextureExists(Guid guid)
        {
            return TextureList.Where(x => x.Value.textureId == guid).Any();
        }

        public static bool DepthTextureExists(Guid guid)
        {
            return DepthTextureList.Where(x => x.Value.textureId == guid).Any();
        }

        public static bool RenderedTextureExists(Guid guid, Guid textureGuid)
        {
            return RenderedTextureListMap[guid].Where(x => x.textureId == textureGuid).Any();
        }

        public static bool RenderedTextureListExists(Guid guid)
        {
            return RenderedTextureListMap[guid].Any();
        }

        public static Texture FindTexture(Guid textureGuid)
        {
            return TextureList[textureGuid];
        }

        public static Texture FindRenderedTexture(Guid textureGuid)
        {
            foreach (var pair in RenderedTextureListMap)
            {
                return pair.Value.Where(x => x.textureId == textureGuid).First();
            }
            return new Texture();
        }

        public static ListPtr<Texture> FindRenderedTextureList(Guid textureGuid)
        {
            return RenderedTextureListMap[textureGuid];
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Guid Texture_LoadTexture(GraphicsRenderer renderer, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_CreateTexture(GraphicsRenderer renderer, Guid textureId, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo, bool useMipMaps);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureSize(GraphicsRenderer renderer, Texture texture, VkImageAspectFlagBits imageType, vec2 TextureResolution);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureBufferIndex(Texture texture, uint bufferIndex);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern bool Texture_TextureExists(Guid guid);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_FindTexture(Guid guid);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_GetTexturePropertiesBuffer(Texture texture, List<VkDescriptorImageInfo> textureDescriptorList);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_DestroyTexture(GraphicsRenderer renderer, Texture texture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateCmdTextureLayout(GraphicsRenderer renderer, VkCommandBuffer commandBuffer, Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint mipmapLevel);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureLayout(GraphicsRenderer renderer, Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint mipmapLevel);
    }
}
