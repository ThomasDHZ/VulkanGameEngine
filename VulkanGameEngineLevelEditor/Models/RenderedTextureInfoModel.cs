using System;
using System.ComponentModel;
using System.Reflection;

namespace VulkanGameEngineLevelEditor.Models
{
    public enum RenderedTextureType
    {
        ColorRenderedTexture,
        DepthRenderedTexture,
        InputAttachmentTexture,
        ResolveAttachmentTexture
    };

    [Serializable]
    public unsafe class RenderedTextureInfoModel
    {
        public Guid TextureId { get; set; }
        public RenderedTextureType TextureType { get; set; }
        public VkImageCreateInfoModel ImageCreateInfo { get; set; } = new VkImageCreateInfoModel();
        public VkSamplerCreateInfoModel SamplerCreateInfo { get; set; } = new VkSamplerCreateInfoModel();
        public VkAttachmentDescriptionModel AttachmentDescription { get; set; } = new VkAttachmentDescriptionModel();

        public RenderedTextureInfoModel() : base()
        {
        }
    }
}
