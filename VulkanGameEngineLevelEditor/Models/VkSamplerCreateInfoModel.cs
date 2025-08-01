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

        [DisplayName("Magnification Filter")]
        [TooltipAttribute("Defines the filter for texture magnification (when scaled up).")]
        public VkFilter MagFilter { get; set; }

        [DisplayName("Minification Filter")]
        [TooltipAttribute("Defines the filter for texture minification (when scaled down).")]
        public VkFilter MinFilter { get; set; }

        [DisplayName("Mip Map Mode")]
        [TooltipAttribute("Specifies how mipmaps are sampled during texture lookup.")]
        public VkSamplerMipmapMode MipmapMode { get; set; }

        [DisplayName("Address Mode U")]
        [TooltipAttribute("Defines texture addressing mode for the U (x) coordinate.")]
        public VkSamplerAddressMode AddressModeU { get; set; }

        [DisplayName("Address Mode V")]
        [TooltipAttribute("Defines texture addressing mode for the V (y) coordinate.")]
        public VkSamplerAddressMode AddressModeV { get; set; }

        [DisplayName("Address Mode W")]
        [TooltipAttribute("Defines texture addressing mode for the W (z) coordinate (used in 3D textures).")]
        public VkSamplerAddressMode AddressModeW { get; set; }

        [DisplayName("Mip Level of Detail")]
        [TooltipAttribute("Adjusts the mipmap level-of-detail bias, influencing mipmap level selection.")]
        public float MipLodBias { get; set; }

        [DisplayName("Anisotrophy Enable")]
        [TooltipAttribute("Enables or disables anisotropic filtering for improved texture quality at oblique angles.")]
        public bool AnisotropyEnable { get; set; }

        [DisplayName("Max Anisotrophy")]
        [TooltipAttribute("Sets the maximum anisotropy level when anisotropic filtering is enabled. Higher values improve quality but may impact performance.")]
        public float MaxAnisotropy { get; set; }

        [DisplayName("Compare Enable")]
        [TooltipAttribute("Enables or disables depth comparison for shadow sampling. Used for percentage-closer filtering.")]
        public bool CompareEnable { get; set; }

        [DisplayName("Compare Operation")]
        [TooltipAttribute("Specifies the comparison operator for depth sampling when depth comparison is enabled.")]
        public VkCompareOp CompareOp { get; set; }

        [DisplayName("Min level of detail")]
        [TooltipAttribute("Sets the minimum level-of-detail, restricting the coarsest mipmap level used during sampling.")]
        public float MinLod { get; set; }

        [DisplayName("Max level of detail")]
        [TooltipAttribute("Sets the maximum level-of-detail, restricting the finest mipmap level used during sampling.")]
        public float MaxLod { get; set; }

        [DisplayName("Border Color")]
        [TooltipAttribute("Defines the color used when texture coordinates are clamped to the border.")]
        public VkBorderColor BorderColor { get; set; }

        [DisplayName("Unnormalized Coordinates")]
        [TooltipAttribute("If true, uses unnormalized texture coordinates (pixel-based); if false, uses normalized coordinates.")]
        public bool UnnormalizedCoordinates { get; set; }

        public VkSamplerCreateInfoModel() : base()
        {
        }
    }
}

