using System.Runtime.InteropServices;
using Vulkan;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkAttachmentReferenceModel
    {
        public uint attachment { get; set; }
        public VkImageLayout layout { get; set; }

        public VkAttachmentReferenceModel()
        { }

        public VkAttachmentReferenceModel(uint attachment, VkImageLayout layout)
        {
            this.attachment = attachment;
            this.layout = layout;
        }
    }
}
