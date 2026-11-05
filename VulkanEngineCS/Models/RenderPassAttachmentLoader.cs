using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;
using VulkanEngineCS.Enum;

namespace VulkanEngineCS.Models
{
    public struct RenderPassAttachmentLoader
    {
        public string AttachmentName = string.Empty;
        public Guid RenderedTextureId { get; set; } = new Guid();
        public uint MipMapCount { get; set; } = uint.MaxValue;
        public TextureTypeEnum TextureType { get; set; } = TextureTypeEnum.kTextureType_Undefined;
        public TextureUsageTypeEnum TextureUsageType { get; set; } = TextureUsageTypeEnum.kUsageType_Undefined;
        public List<RenderAttachmentTypeEnum> RenderAttachmentTypes { get; set; } = new List<RenderAttachmentTypeEnum>();
        public VkFormat TextureByteFormat { get; set; } = VkFormat.VK_FORMAT_R8G8B8A8_UNORM;
        public VkAttachmentLoadOp LoadOp { get; set; } = VkAttachmentLoadOp.VK_ATTACHMENT_LOAD_OP_CLEAR;
        public VkAttachmentStoreOp StoreOp { get; set; } = VkAttachmentStoreOp.VK_ATTACHMENT_STORE_OP_STORE;
        public VkSamplerCreateInfo SamplerCreateInfo { get; set; } = new VkSamplerCreateInfo();
        public VkImageLayout FinalLayout { get; set; } = VkImageLayout.VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        public VkSampleCountFlagBits SampleCount { get; set; } = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT;
        public bool UseMipMaps { get; set; } = false;
        public bool IsSkyBox { get; set; } = false;

        public RenderPassAttachmentLoader()
        {
        }
    }
}
