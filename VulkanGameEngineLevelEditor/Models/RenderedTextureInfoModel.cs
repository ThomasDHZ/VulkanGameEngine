using System;
using System.ComponentModel;
using System.Reflection;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

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
        [IgnoreProperty]
        [Tooltip("Unique identifier for the texture.")]
        public Guid TextureId { get; set; }

        [DisplayName("Texture Type")]
        [Tooltip("Specifies the type of the rendered texture.")]
        public RenderedTextureType TextureType { get; set; }

        [DisplayName("Texture Properties Info")]
        [Tooltip("Defines the properties for creating the texture's image.")]
        public VkImageCreateInfoModel ImageCreateInfo { get; set; } = new VkImageCreateInfoModel();

        [DisplayName("Sampler Info")]
        [Tooltip("Configures the sampler for accessing the texture.")]
        public VkSamplerCreateInfoModel SamplerCreateInfo { get; set; } = new VkSamplerCreateInfoModel();

        [DisplayName("Attachment Description")]
        [Tooltip("Describes the attachment properties for the render pass.")]
        public VkAttachmentDescriptionModel AttachmentDescription { get; set; } = new VkAttachmentDescriptionModel();

        public RenderedTextureInfoModel() : base()
        {
        }
    }
}
