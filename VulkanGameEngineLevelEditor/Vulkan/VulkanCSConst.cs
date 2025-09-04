using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Vulkan
{
    public class VulkanCSConst
    {
        public const bool VK_FALSE = false;
        public const bool VK_TRUE = true;
        public const uint VK_MAX_MEMORY_TYPES = 32;
        public const uint VK_MAX_MEMORY_HEAPS = 16;
        public const uint VK_MAX_PHYSICAL_DEVICE_NAME_SIZE = 256;
        public const uint VK_UUID_SIZE = 16;
        public const uint VK_MAX_EXTENSION_NAME_SIZE = 256;
        public const uint VK_MAX_DESCRIPTION_SIZE = 256;
        public const uint VK_QUEUE_FAMILY_IGNORED = uint.MaxValue;
        public const uint VK_SUBPASS_EXTERNAL = uint.MaxValue;
        public const uint MAX_FRAMES_IN_FLIGHT = 3;
        public static readonly IntPtr VK_NULL_HANDLE = IntPtr.Zero;
        public static readonly ulong VK_WHOLE_SIZE = ulong.MaxValue;
    }
}
