using GlmSharp;
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
    public unsafe class VkImageCreateInfoModel
    {
        [IgnoreProperty]
        [TooltipAttribute("Specifies the Vulkan structure type. Must be set to the image creation type.")]
        public VkStructureType SType { get; set; } = VkStructureType.VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Specifies flags for image creation, such as sparse binding or mutable format.")]
        public VkImageCreateFlagBits Flags { get; set; } = 0;

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Pointer to an extension structure for additional image parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }


        [DisplayName("Image Type")]
        [TooltipAttribute("Defines the type of image, such as 1D, 2D, or 3D.")]
        public VkImageType ImageType { get; set; }

        [TooltipAttribute("Specifies the format and data type of the image's texels.")]
        public VkFormat Format { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [DisplayName("Texture Size")]
        [TooltipAttribute("Sets the dimensions of the image (width, height, depth).")]
        public VkExtent3DModel Extent { get; set; } = new VkExtent3DModel();

        [DisplayName("MipMap Levels")]
        [TooltipAttribute("Specifies the number of mipmap levels for the image.")]
        public uint MipLevels { get; set; }

        [DisplayName("Array Layers")]
        [TooltipAttribute("Specifies the number of array layers for the image, used for array textures.")]
        public uint ArrayLayers { get; set; }

        [DisplayName("Sample Count")]
        [TooltipAttribute("Defines the number of samples per texel for multisampling.")]
        public VkSampleCountFlagBits Samples { get; set; }

        [TooltipAttribute("Specifies the tiling arrangement of image data, affecting memory layout.")]
        public VkImageTiling Tiling { get; set; }

        [TooltipAttribute("Defines the intended usage of the image, such as rendering or sampling.")]
        public VkImageUsageFlagBits Usage { get; set; }

        [DisplayName("Sharing Mode")]
        [TooltipAttribute("Specifies how the image is shared between queue families.")]
        public VkSharingMode SharingMode { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Sets the number of queue family indices for sharing the image.")]
        public uint QueueFamilyIndexCount { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [TooltipAttribute("Pointer to an array of queue family indices for image sharing.")]
        public unsafe uint* PQueueFamilyIndices { get; set; }

        [DisplayName("Start Layout")]
        [TooltipAttribute("Specifies the initial layout of the image after creation.")]
        public VkImageLayout InitialLayout { get; set; }

        public VkImageCreateInfoModel() : base()
        {
        }
    }
}
