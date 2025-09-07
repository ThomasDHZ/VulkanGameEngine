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

        public static void UpdateTextureLayout(Texture texture, VkImageLayout newImageLayout)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, texture.textureImageLayout, newImageLayout);
        }

        public static void UpdateTextureLayout(Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            Texture_UpdateTextureLayout(RenderSystem.renderer, texture, oldImageLayout, newImageLayout);
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout newImageLayout)
        {
            VkImageMemoryBarrier imageMemoryBarrier = new VkImageMemoryBarrier
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                srcAccessMask = 0,
                dstAccessMask = VkAccessFlagBits.VK_ACCESS_TRANSFER_WRITE_BIT,
                oldLayout = texture.textureImageLayout,
                newLayout = newImageLayout,
                image = texture.textureImage,
                subresourceRange =

                    {
                        aspectMask = VkImageAspectFlagBits.VK_IMAGE_ASPECT_COLOR_BIT,
                        baseMipLevel = texture.mipMapLevels - 1,
                        levelCount = VulkanCSConst.VK_REMAINING_MIP_LEVELS,
                        layerCount = 1
                }
            };

            VkFunc.vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits.VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlagBits.VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, null, 0, null, 1, &imageMemoryBarrier);
            texture.textureImageLayout = newImageLayout;
        }

        public static void UpdateTextureLayout(Texture texture, VkCommandBuffer commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
        {
            Texture_UpdateCmdTextureLayout(RenderSystem.renderer, commandBuffer, texture, oldImageLayout, newImageLayout);
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

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_LoadTexture(GraphicsRenderer renderer, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Texture Texture_CreateTexture(GraphicsRenderer renderer, VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureSize(GraphicsRenderer renderer, Texture texture, VkImageAspectFlagBits imageType, vec2 TextureResolution);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureBufferIndex(Texture texture, uint bufferIndex);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_GetTexturePropertiesBuffer(Texture texture, List<VkDescriptorImageInfo> textureDescriptorList);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_DestroyTexture(GraphicsRenderer renderer, Texture texture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateCmdTextureLayout(GraphicsRenderer renderer, VkCommandBuffer commandBuffer, Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Texture_UpdateTextureLayout(GraphicsRenderer renderer, Texture texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    }
}
