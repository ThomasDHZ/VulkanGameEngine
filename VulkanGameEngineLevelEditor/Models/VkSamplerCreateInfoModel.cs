using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class VkSamplerCreateInfoModel : RenderPassEditorBaseModel
    {
        [IgnoreProperty]
        [TooltipAttribute("Specifies the Vulkan structure type. Must be set to the sampler creation type.")]
        public VkStructureType SType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Reserved for future use. Currently set to 0, as no flags are defined.")]
        public VkSamplerCreateFlagBits Flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Pointer to an extension structure for additional sampler parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }

        [TooltipAttribute("Defines the filter for texture magnification (when scaled up).")]
        public VkFilter MagFilter { get; set; }

        [TooltipAttribute("Defines the filter for texture minification (when scaled down).")]
        public VkFilter MinFilter { get; set; }

        [TooltipAttribute("Specifies how mipmaps are sampled during texture lookup.")]
        public VkSamplerMipmapMode MipmapMode { get; set; }

        [TooltipAttribute("Defines texture addressing mode for the U (x) coordinate.")]
        public VkSamplerAddressMode AddressModeU { get; set; }

        [TooltipAttribute("Defines texture addressing mode for the V (y) coordinate.")]
        public VkSamplerAddressMode AddressModeV { get; set; }

        [TooltipAttribute("Defines texture addressing mode for the W (z) coordinate (used in 3D textures).")]
        public VkSamplerAddressMode AddressModeW { get; set; }

        [TooltipAttribute("Adjusts the mipmap level-of-detail bias, influencing mipmap level selection.")]
        public float MipLodBias { get; set; }

        [TooltipAttribute("Enables or disables anisotropic filtering for improved texture quality at oblique angles.")]
        public bool AnisotropyEnable { get; set; }

        [TooltipAttribute("Sets the maximum anisotropy level when anisotropic filtering is enabled. Higher values improve quality but may impact performance.")]
        public float MaxAnisotropy { get; set; }

        [TooltipAttribute("Enables or disables depth comparison for shadow sampling. Used for percentage-closer filtering.")]
        public bool CompareEnable { get; set; }

        [TooltipAttribute("Specifies the comparison operator for depth sampling when depth comparison is enabled.")]
        public VkCompareOp CompareOp { get; set; }

        [TooltipAttribute("Sets the minimum level-of-detail, restricting the coarsest mipmap level used during sampling.")]
        public float MinLod { get; set; }

        [TooltipAttribute("Sets the maximum level-of-detail, restricting the finest mipmap level used during sampling.")]
        public float MaxLod { get; set; }

        [TooltipAttribute("Defines the color used when texture coordinates are clamped to the border.")]
        public VkBorderColor BorderColor { get; set; }

        [TooltipAttribute("If true, uses unnormalized texture coordinates (pixel-based); if false, uses normalized coordinates.")]
        public bool UnnormalizedCoordinates { get; set; }

        public VkSamplerCreateInfoModel() : base()
        {
        }
    }
}

