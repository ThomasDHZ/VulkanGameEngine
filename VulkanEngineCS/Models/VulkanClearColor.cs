using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCS.Models
{
    public enum VulkanClearColorEnum
    {
        kClearColor,
        kDepthColor
    }
    public struct VulkanClearColor
    {
        public VulkanClearColorEnum ClearColorType { get; set; }
        public VkClearValue ClearValue { get; set; }
    }
}
