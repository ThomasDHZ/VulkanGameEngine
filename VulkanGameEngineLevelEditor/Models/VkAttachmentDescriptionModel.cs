using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public class VkAttachmentDescriptionModel
    {
        [IgnoreProperty]
        [Tooltip("Specifies the Vulkan structure type. Must be set to the attachment description type.")]
        public VkStructureType StructureType { get; set; }

        [IgnoreProperty]
        [Tooltip("Specifies flags for the attachment, such as aliasing behavior.")]
        public VkAttachmentDescriptionFlagBits Flags { get; set; }

        [JsonIgnore]
        [IgnoreProperty]
        [Tooltip("Pointer to an extension structure for additional attachment parameters. Typically null unless using extensions.")]
        public IntPtr pNext { get; set; }

        [Tooltip("Specifies the format of the attachment's image data.")]
        public VkFormat Format { get; set; }

        [Tooltip("Defines the number of samples per pixel for the attachment.")]
        public VkSampleCountFlagBits Samples { get; set; }

        [DisplayName("Load Opepration")]
        [Tooltip("Specifies how the attachment's color or depth data is loaded at the start of a render pass.")]
        public VkAttachmentLoadOp LoadOp { get; set; }

        [DisplayName("Store Opperation")]
        [Tooltip("Specifies how the attachment's color or depth data is stored at the end of a render pass.")]
        public VkAttachmentStoreOp StoreOp { get; set; }

        [DisplayName("Stencil Load Opperation")]
        [Tooltip("Specifies how the attachment's stencil data is loaded at the start of a render pass.")]
        public VkAttachmentLoadOp StencilLoadOp { get; set; }

        [DisplayName("Stencil Store Opperation")]
        [Tooltip("Specifies how the attachment's stencil data is stored at the end of a render pass.")]
        public VkAttachmentStoreOp StencilStoreOp { get; set; }

        [DisplayName("Start Layout")]
        [Tooltip("Defines the initial layout of the attachment's image before the render pass.")]
        public VkImageLayout InitialLayout { get; set; }

        [DisplayName("Final Layout")]
        [Tooltip("Defines the final layout of the attachment's image after the render pass.")]
        public VkImageLayout FinalLayout { get; set; }
    }
}
