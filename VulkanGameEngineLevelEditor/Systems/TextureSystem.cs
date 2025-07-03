﻿using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
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
using VulkanGameEngineLevelEditor.GameEngineAPI;

namespace VulkanGameEngineLevelEditor.Systems
{
    public unsafe static class TextureSystem
    {
        public static Dictionary<Guid, Texture> TextureList { get; set; } = new Dictionary<Guid, Texture>();
        public static Dictionary<Guid, Texture> DepthTextureList { get; set; } = new Dictionary<Guid, Texture>();
        public static Dictionary<Guid, ListPtr<Texture>> RenderedTextureList { get; set; } = new Dictionary<Guid, ListPtr<Texture>>();
        public static Dictionary<uint, ListPtr<Texture>> InputTextureList { get; set; } = new Dictionary<uint, ListPtr<Texture>>();

        public static Guid LoadTexture(string texturePath)
        {
            if (texturePath.IsEmpty())
            {
                return new Guid();
            }

            string jsonContent = File.ReadAllText(texturePath);
            TextureJsonLoader textureJson = JsonConvert.DeserializeObject<TextureJsonLoader>(jsonContent);
            if (TextureList.ContainsKey(textureJson.TextureId))
            {
                return textureJson.TextureId;
            }

            TextureList[textureJson.TextureId] = Texture_LoadTexture(RenderSystem.renderer, texturePath);
            return textureJson.TextureId;
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

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_LoadTexture( GraphicsRenderer renderer, [MarshalAs(UnmanagedType.LPStr)] string jsonString);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_CreateTexture( GraphicsRenderer renderer, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureSize( GraphicsRenderer renderer, Texture texture, VkImageAspectFlagBits imageType, vec2 TextureResolution);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureBufferIndex(Texture texture, uint bufferIndex);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_GetTexturePropertiesBuffer(Texture texture, List<VkDescriptorImageInfo> textureDescriptorList);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_DestroyTexture( GraphicsRenderer renderer, Texture texture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateCmdTextureLayout( GraphicsRenderer renderer, VkCommandBuffer commandBuffer, Texture texture, VkImageLayout newImageLayout);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureLayout( GraphicsRenderer renderer, Texture texture, VkImageLayout newImageLayout);
    }
}
