using Newtonsoft.Json;
using System;
using System.ComponentModel;
using System.Reflection;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public class VkAttachmentDescriptionModel
    {
        public VkStructureType StructureType { get; set; }
        public VkAttachmentDescriptionFlagBits Flags { get; set; }
        [JsonIgnore]
        public IntPtr pNext { get; set; }
        public VkFormat Format { get; set; }
        public VkSampleCountFlagBits Samples { get; set; }
        public VkAttachmentLoadOp LoadOp { get; set; }
        public VkAttachmentStoreOp StoreOp { get; set; }
        public VkAttachmentLoadOp StencilLoadOp { get; set; }
        public VkAttachmentStoreOp StencilStoreOp { get; set; }
        public VkImageLayout InitialLayout { get; set; }
        public VkImageLayout FinalLayout { get; set; }
    }
}
