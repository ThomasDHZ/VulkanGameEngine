﻿using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using StbImageSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using VulkanGameEngineLevelEditor;
using VulkanGameEngineLevelEditor.Models;
using VulkanGameEngineLevelEditor.Vulkan;
using static System.Windows.Forms.DataFormats;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public unsafe class RenderedTexture : Texture
    {
        public RenderedTexture() : base()
        {

        }

        public RenderedTexture(VkImageAspectFlagBits imageType, VkImageCreateInfo createImageInfo, VkSamplerCreateInfo samplerCreateInfo) : base()
        {
            Width = (int)createImageInfo.extent.width;
            Height = (int)createImageInfo.extent.height;
            Depth = (int)createImageInfo.extent.depth;
            TextureByteFormat = createImageInfo.format;
            TextureImageLayout = createImageInfo.initialLayout;
            SampleCount = createImageInfo.samples;

            CreateImage(createImageInfo);
            CreateTextureView(imageType);
            CreateTextureSampler(samplerCreateInfo);

        }

        public void RecreateRendererTexture(VkImageAspectFlagBits imageType, vec2 TextureResolution)
        {
            Width = (int)TextureResolution.x;
            Height = (int)TextureResolution.y;

            base.Destroy();

            VkImageCreateInfo imageCreateInfo = new VkImageCreateInfo
            {
		        sType = VkStructureType.VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		        imageType = VkImageType.VK_IMAGE_TYPE_2D,
		        format = TextureByteFormat,
		        extent =
                {
			        width = (uint)Width,
			        height = (uint)Height,
			        depth = 1
                },
		        mipLevels = MipMapLevels,
		        arrayLayers = 1,
		        samples = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT,
		        tiling = VkImageTiling.VK_IMAGE_TILING_OPTIMAL,
		        usage = VkImageUsageFlagBits.VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
                        VkImageUsageFlagBits.VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                        VkImageUsageFlagBits.VK_IMAGE_USAGE_SAMPLED_BIT |
                        VkImageUsageFlagBits.VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		        sharingMode = VkSharingMode.VK_SHARING_MODE_EXCLUSIVE,
		        initialLayout = VkImageLayout.VK_IMAGE_LAYOUT_UNDEFINED,
            };


            CreateImage(imageCreateInfo);
            CreateTextureView(imageType);
            CreateTextureSampler();
        }
    }
}
