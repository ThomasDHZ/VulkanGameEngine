using System;
using System.ComponentModel;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.Json.Serialization;
using System.Threading;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using static System.Net.Mime.MediaTypeNames;

namespace Vulkan
{
    using HINSTANCE = nint;
    using HWND = nint;
    using size_t = nint;
    using VkBuffer = nint;
    using VkBufferView = nint;
    using VkCommandBuffer = nint;
    using VkCommandPool = nint;
    using VkDescriptorPool = nint;
    using VkDescriptorSet = nint;
    using VkDescriptorSetLayout = nint;
    using VkDeviceMemory = nint;
    using VkDeviceSize = System.UInt64;
    using VkFramebuffer = nint;
    using VkImage = nint;
    using VkImageView = nint;
    using VkPipeline = nint;
    using VkPipelineLayout = nint;
    using VkRenderPass = nint;
    using VkSampleMask = uint;
    using VkSampler = nint;
    using VkSemaphore = nint;
    using VkShaderModule = nint;
    using VkSubpassDescriptionFlags = uint;
    using VkSwapchainKHR = nint;
    using VkWin32SurfaceCreateFlagsKHR = uint;

    [StructLayout(LayoutKind.Sequential)]
    public struct VkAttachmentReference
    {
        public uint attachment;
        public VkImageLayout layout;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct VkStencilOpState
    {
        public int failOp;  // VkStencilOp (enum int)
        public int passOp;
        public int depthFailOp;
        public int compareOp;  // VkCompareOp
        public uint compareMask;
        public uint writeMask;
        public uint reference;

        public VkStencilOpState()
        {
            failOp = 1;  // VK_STENCIL_OP_KEEP (default from logs)
            passOp = 1;
            depthFailOp = 1;
            compareOp = 0;  // VK_COMPARE_OP_NEVER (or ALWAYS=7 if needed)
            compareMask = 0;
            writeMask = 0;
            reference = 0;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSubpassDependency
    {
        public uint srcSubpass;
        public uint dstSubpass;
        public VkPipelineStageFlagBits srcStageMask;
        public VkPipelineStageFlagBits dstStageMask;
        public VkAccessFlagBits srcAccessMask;
        public VkAccessFlagBits dstAccessMask;
        public VkDependencyFlagBits dependencyFlags;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineDepthStencilStateCreateInfo
    {
        public int sType;  // VkStructureType
        public void* pNext;
        public uint flags;  // VkPipelineDepthStencilStateCreateFlags (uint)
        public uint depthTestEnable;  // VkBool32
        public uint depthWriteEnable;  // VkBool32
        public int depthCompareOp;  // VkCompareOp (enum int)
        public uint depthBoundsTestEnable;  // VkBool32
        public uint stencilTestEnable;  // VkBool32
        public VkStencilOpState front;  // Inline struct (define below; zero if unused)
        public VkStencilOpState back;   // Inline
        public float minDepthBounds;
        public float maxDepthBounds;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineColorBlendStateCreateInfo
    {
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        public struct Blender
        {
            public float R;
            public float G;
            public float B;
            public float A;

            public Blender()
            {
                R = 0f;
                G = 0f;
                B = 0f;
                A = 0f;
            }
        }

        public int sType;  // VkStructureType
        public void* pNext;
        public uint flags;  // VkPipelineColorBlendStateCreateFlags (uint)
        public uint logicOpEnable;  // VkBool32
        public int logicOp;  // VkLogicOp (enum int)
        public uint attachmentCount;
        public VkPipelineColorBlendAttachmentState* pAttachments;  // Pin array separately
        public Blender blendConstants;

        public VkPipelineColorBlendStateCreateInfo()
        {
            sType = 1000000000 + 12;  // VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
            pNext = null;
            flags = 0;
            logicOpEnable = 0;
            logicOp = 0;  // VK_LOGIC_OP_CLEAR
            attachmentCount = 0;
            pAttachments = null;
            blendConstants = new Blender();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkAttachmentDescription
    {
        public VkAttachmentDescriptionFlagBits flags;
        public VkFormat format;
        public VkSampleCountFlagBits samples;
        public VkAttachmentLoadOp loadOp;
        public VkAttachmentStoreOp storeOp;
        public VkAttachmentLoadOp stencilLoadOp;
        public VkAttachmentStoreOp stencilStoreOp;
        public VkImageLayout initialLayout;
        public VkImageLayout finalLayout;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineMultisampleStateCreateInfo
    {
        public int sType;  // VkStructureType
        public void* pNext;
        public uint flags;  // VkPipelineMultisampleStateCreateFlags (uint)
        public int rasterizationSamples;  // VkSampleCountFlagBits (enum int)
        public uint sampleShadingEnable;  // VkBool32
        public float minSampleShading;
        public void* pSampleMask;  // const VkSampleMask* (use void* or fixed uint[1]; set to null for none)
        public uint alphaToCoverageEnable;  // VkBool32
        public uint alphaToOneEnable;  // VkBool32

        public VkPipelineMultisampleStateCreateInfo()
        {
            sType = 1000000000 + 11;  // VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
            pNext = null;
            flags = 0;
            rasterizationSamples = 1;  // VK_SAMPLE_COUNT_1_BIT
            sampleShadingEnable = 0;
            minSampleShading = 0f;
            pSampleMask = null;
            alphaToCoverageEnable = 0;
            alphaToOneEnable = 0;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSamplerCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkSamplerCreateFlagBits flags;
        public VkFilter magFilter;
        public VkFilter minFilter;
        public VkSamplerMipmapMode mipmapMode;
        public VkSamplerAddressMode addressModeU;
        public VkSamplerAddressMode addressModeV;
        public VkSamplerAddressMode addressModeW;
        public float mipLodBias;
        public VkBool32 anisotropyEnable;
        public float maxAnisotropy;
        public VkBool32 compareEnable;
        public VkCompareOp compareOp;
        public float minLod;
        public float maxLod;
        public VkBorderColor borderColor;
        public VkBool32 unnormalizedCoordinates;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkViewport
    {
        [Tooltip("Specifies the x-coordinate of the viewport’s upper-left corner in pixels.")]
        public float x { get; set; }

        [Tooltip("Specifies the y-coordinate of the viewport’s upper-left corner in pixels.")]
        public float y { get; set; }

        [Tooltip("Defines the width of the viewport in pixels.")]
        public float width { get; set; }

        [Tooltip("Defines the height of the viewport in pixels.")]
        public float height { get; set; }

        [DisplayName("Max Depth")]
        [Tooltip("Sets the minimum depth value for the viewport, typically between 0.0 and 1.0.")]
        public float minDepth { get; set; }

        [DisplayName("Min Depth")]
        [Tooltip("Sets the maximum depth value for the viewport, typically between 0.0 and 1.0.")]
        public float maxDepth { get; set; }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkOffset2D
    {
        [Tooltip("Specifies the x-coordinate offset in pixels.")]
        public int x { get; set; }

        [Tooltip("Specifies the y-coordinate offset in pixels.")]
        public int y { get; set; }

        public VkOffset2D(int x, int y) : this()
        {
            this.x = x;
            this.y = y;
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct VkRect2D
    {
        [Tooltip("Specifies the 2D offset of the rectangle’s top-left corner in pixels.")]
        public VkOffset2D offset { get; set; }

        [Tooltip("Defines the 2D extent (width and height) of the rectangle in pixels.")]
        public VkExtent2D extent { get; set; }

        public VkRect2D(VkOffset2D vkOffset2D, VkExtent2D swapChainResolution) : this()
        {
            this.offset = vkOffset2D;
            this.extent = swapChainResolution;
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkDescriptorSetLayoutBinding
    {
        public uint binding;
        public VkDescriptorType descriptorType;
        public uint descriptorCount;
        public VkShaderStageFlagBits stageFlags;
        public VkSampler* pImmutableSamplers;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineRasterizationStateCreateInfo
    {
        public int sType;  // VkStructureType
        public void* pNext;
        public uint flags;  // VkPipelineRasterizationStateCreateFlags (uint)
        public uint depthClampEnable;  // VkBool32
        public uint rasterizerDiscardEnable;  // VkBool32
        public int polygonMode;  // VkPolygonMode (enum int)
        public uint cullMode;  // VkCullModeFlags (uint bitmask)
        public int frontFace;  // VkFrontFace (enum int)
        public uint depthBiasEnable;  // VkBool32
        public float depthBiasConstantFactor;
        public float depthBiasClamp;
        public float depthBiasSlopeFactor;
        public float lineWidth;

        public VkPipelineRasterizationStateCreateInfo()
        {
            sType = 1000000000 + 10;  // VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
            pNext = null;
            flags = 0;
            depthClampEnable = 0;
            rasterizerDiscardEnable = 0;
            polygonMode = 1;  // VK_POLYGON_MODE_FILL
            cullMode = 2;  // VK_CULL_MODE_BACK_BIT
            frontFace = 1;  // VK_FRONT_FACE_COUNTER_CLOCKWISE
            depthBiasEnable = 0;
            depthBiasConstantFactor = 0f;
            depthBiasClamp = 0f;
            depthBiasSlopeFactor = 0f;
            lineWidth = 1f;
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct VkPipelineColorBlendAttachmentState
    {
        public uint blendEnable;  // VkBool32
        public int srcColorBlendFactor;  // VkBlendFactor (enum int)
        public int dstColorBlendFactor;  // VkBlendFactor
        public int colorBlendOp;  // VkBlendOp
        public int srcAlphaBlendFactor;  // VkBlendFactor
        public int dstAlphaBlendFactor;  // VkBlendFactor
        public int alphaBlendOp;  // VkBlendOp
        public uint colorWriteMask;  // VkColorComponentFlags (uint bitmask)

        public VkPipelineColorBlendAttachmentState()
        {
            blendEnable = 0;  // VK_FALSE
            srcColorBlendFactor = 0;  // Default to VK_BLEND_FACTOR_ONE or set explicitly
            dstColorBlendFactor = 0;  // VK_BLEND_FACTOR_ZERO
            colorBlendOp = 0;  // VK_BLEND_OP_ADD
            srcAlphaBlendFactor = 0;
            dstAlphaBlendFactor = 0;
            alphaBlendOp = 0;  // VK_BLEND_OP_ADD (fix invalid 15)
            colorWriteMask = 0xF;  // All components (R|G|B|A=15)
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineInputAssemblyStateCreateInfo
    {
        public int sType;  // VkStructureType (enum int)
        public void* pNext;
        public uint flags;  // VkPipelineInputAssemblyStateCreateFlags (uint)
        public int topology;  // VkPrimitiveTopology (enum int)
        public uint primitiveRestartEnable;  // VkBool32

        public VkPipelineInputAssemblyStateCreateInfo()
        {
            sType = 1000000000 + 7;  // VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO (adjust to actual value)
            pNext = null;
            flags = 0;
            topology = 3;  // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST (default from logs)
            primitiveRestartEnable = 0;  // VK_FALSE
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkExtent2D
    {
        [Tooltip("Defines the width of the 2D extent in pixels.")]
        public uint width { get; set; }

        [Tooltip("Defines the height of the 2D extent in pixels.")]
        public uint height { get; set; }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkExtent3D
    {
        public uint width;
        public uint height;
        public uint depth;

        public VkExtent3D(uint width, uint height, uint depth) : this()
        {
            this.width = width;
            this.height = height;
            this.depth = depth;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkImageCreateInfo
    {
        public VkStructureType sType = VkStructureType.VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        public void* pNext;
        public VkImageCreateFlagBits flags;
        public VkImageType imageType;
        public VkFormat format;
        public VkExtent3D extent;
        public uint mipLevels;
        public uint arrayLayers;
        public VkSampleCountFlagBits samples;
        public VkImageTiling tiling;
        public VkImageUsageFlagBits usage;
        public VkSharingMode sharingMode;
        public uint queueFamilyIndexCount;
        public uint* pQueueFamilyIndices;
        public VkImageLayout initialLayout;

        public VkImageCreateInfo()
        {
        }

        public int UiPropertiesControls(object obj, PropertyInfo property, int xPosition, int yOffset, int width)
        {

            return yOffset;
        }
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSurfaceCapabilitiesKHR
    {
        public uint minImageCount;
        public uint maxImageCount;
        public VkExtent2D currentExtent;
        public VkExtent2D minImageExtent;
        public VkExtent2D maxImageExtent;
        public uint maxImageArrayLayers;
        public VkSurfaceTransformFlagBitsKHR supportedTransforms;
        public VkSurfaceTransformFlagBitsKHR currentTransform;
        public VkCompositeAlphaFlagBitsKHR supportedCompositeAlpha;
        public VkImageUsageFlagBits supportedUsageFlags;
    };

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkFramebufferCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkFramebufferCreateFlagBits flags;
        public VkRenderPass renderPass;
        public uint attachmentCount;
        public VkImageView* pAttachments;
        public uint width;
        public uint height;
        public uint layers;
    };

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkCommandBufferBeginInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkCommandBufferUsageFlagBits flags;
        public VkCommandBufferInheritanceInfo* pInheritanceInfo;
    }


    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSemaphoreTypeCreateInfo
    {
        public VkStructureType sType;                     // Structure type
        public void* pNext;                              // Pointer to extension-specific structures (NULL for none)
        public VkSemaphoreType semaphoreType;             // Type of semaphore (binary or timeline)
        public ulong initialValue;                         // Initial value for the semaphore (only relevant for timeline semaphores)
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkWin32SurfaceCreateInfoKHR
    {
        public VkStructureType sType;
        public void* pNext;
        public VkWin32SurfaceCreateFlagsKHR flags;
        public HINSTANCE hinstance;
        public HWND hwnd;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkCommandBufferInheritanceInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkRenderPass renderPass;
        public uint subpass;
        public VkFramebuffer framebuffer;
        public VkBool32 occlusionQueryEnable;
        public VkQueryControlFlagBits queryFlags;
        public VkQueryPipelineStatisticFlagBits pipelineStatistics;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkRenderPassBeginInfo
    {
        public VkStructureType sType { get; set; }
        public IntPtr pNext { get; set; }
        public VkRenderPass renderPass { get; set; }
        public VkFramebuffer framebuffer { get; set; }
        public VkRect2D renderArea { get; set; }
        public size_t clearValueCount { get; set; }
        public VkClearValue* pClearValues { get; set; }
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkClearColorValue
    {

        public float Float32_0 { get; set; } = 0.0f;
        public float Float32_1 { get; set; } = 0.0f;
        public float Float32_2 { get; set; } = 0.0f;
        public float Float32_3 { get; set; } = 0.0f;
        [IgnoreProperty]
        public int Int32_0 { get; set; } = 0;
        [IgnoreProperty]
        public int Int32_1 { get; set; } = 0;
        [IgnoreProperty]
        public int Int32_2 { get; set; } = 0;
        [IgnoreProperty]
        public int Int32_3 { get; set; } = 0;
        [IgnoreProperty]
        public uint Uint32_0 { get; set; } = 0;
        [IgnoreProperty]
        public uint Uint32_1 { get; set; } = 0;
        [IgnoreProperty]
        public uint Uint32_2 { get; set; } = 0;
        [IgnoreProperty]
        public uint Uint32_3 { get; set; } = 0;
        public VkClearColorValue()
        {
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    public unsafe partial struct VkClearValue
    {
        public VkClearValue
        (
            VkClearColorValue? color = null,
            VkClearDepthStencilValue? depthStencil = null
        ) : this()
        {
            if (color is not null)
            {
                Color = color.Value;
            }

            if (depthStencil is not null)
            {
                DepthStencil = depthStencil.Value;
            }
        }

        [FieldOffset(0)]
        public VkClearColorValue Color;

        [FieldOffset(0)]
        public VkClearDepthStencilValue DepthStencil;
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct VkClearDepthStencilValue
    {
        public float depth;
        public uint stencil;

        public VkClearDepthStencilValue(float depth, uint stencil) : this()
        {
            this.depth = depth;
            this.stencil = stencil;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSparseImageFormatProperties
    {
        public VkImageAspectFlagBits aspectMask;
        public VkExtent3D imageGranularity;
        public VkSparseImageFormatFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkFenceCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkFenceCreateFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkAllocationCallbacks
    {
        public void* pUserData;
        public IntPtr pfnAllocation;
        public IntPtr pfnReallocation;
        public IntPtr pfnFree;
        public IntPtr pfnInternalAllocation;
        public IntPtr pfnInternalFree;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSparseMemoryBind
    {
        public VkDeviceSize resourceOffset;
        public VkDeviceSize size;
        public VkDeviceMemory memory;
        public VkDeviceSize memoryOffset;
        public VkSparseMemoryBindFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSparseBufferMemoryBindInfo
    {
        public VkBuffer buffer;
        public uint bindCount;
        public VkSparseMemoryBind* pBinds;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSparseImageOpaqueMemoryBindInfo
    {
        public VkImage image;
        public uint bindCount;
        public VkSparseMemoryBind* pBinds;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSparseImageMemoryBindInfo
    {
        public VkImage image;
        public uint bindCount;
        public VkSparseImageMemoryBind* pBinds;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkOffset3D
    {
        public int x;
        public int y;
        public int z;
        public VkOffset3D() { }
        public VkOffset3D(int x, int y, int z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSparseImageMemoryBind
    {
        public VkImageSubresource subresource;
        public VkOffset3D offset;
        public VkExtent3D extent;
        public VkDeviceMemory memory;
        public VkDeviceSize memoryOffset;
        public VkSparseMemoryBindFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageSubresource
    {
        public VkImageAspectFlagBits aspectMask;
        public uint mipLevel;
        public uint arrayLayer;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkBindSparseInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public uint waitSemaphoreCount;
        public VkSemaphore* pWaitSemaphores;
        public uint bufferBindCount;
        public VkSparseBufferMemoryBindInfo* pBufferBinds;
        public uint imageOpaqueBindCount;
        public VkSparseImageOpaqueMemoryBindInfo* pImageOpaqueBinds;
        public uint imageBindCount;
        public VkSparseImageMemoryBindInfo* pImageBinds;
        public uint signalSemaphoreCount;
        public VkSemaphore* pSignalSemaphores;
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct VkSparseImageMemoryRequirements
    {
        public VkSparseImageFormatProperties formatProperties;
        public uint imageMipTailFirstLod;
        public VkDeviceSize imageMipTailSize;
        public VkDeviceSize imageMipTailOffset;
        public VkDeviceSize imageMipTailStride;
    }

    //[StructLayout(LayoutKind.Sequential)]
    //public unsafe struct VkClearValue
    //{
    //    public VkClearColorValue color;
    //    public VkClearDepthStencilValue depthStencil;
    //}

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkMemoryRequirements
    {
        public VkDeviceSize size;
        public VkDeviceSize alignment;
        public uint memoryTypeBits;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSubpassDescription
    {
        public VkSubpassDescriptionFlags flags;
        public VkPipelineBindPoint pipelineBindPoint;
        public uint inputAttachmentCount;
        public VkAttachmentReference* pInputAttachments;
        public uint colorAttachmentCount;
        public VkAttachmentReference* pColorAttachments;
        public VkAttachmentReference* pResolveAttachments;
        public VkAttachmentReference* pDepthStencilAttachment;
        public uint preserveAttachmentCount;
        public uint* pPreserveAttachments;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkRenderPassCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkRenderPassCreateFlags flags;
        public uint attachmentCount;
        public VkAttachmentDescription* pAttachments;
        public uint subpassCount;
        public VkSubpassDescription* pSubpasses;
        public uint dependencyCount;
        public VkSubpassDependency* pDependencies;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkDescriptorPoolSize
    {
        public VkDescriptorType type;
        public uint descriptorCount;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkDescriptorPoolCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkDescriptorPoolCreateFlagBits flags;
        public uint maxSets;
        public uint poolSizeCount;
        public VkDescriptorPoolSize* pPoolSizes;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VkPipelineViewportStateCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkCullModeFlagBits flags;
        public uint viewportCount;
        public VkViewport* pViewports;
        public uint scissorCount;
        public VkRect2D* pScissors;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkVertexInputBindingDescription
    {
        [Tooltip("Specifies the binding number for the vertex input.")]
        public uint binding { get; set; }

        [Tooltip("Defines the stride in bytes between consecutive vertex data entries.")]
        public uint stride { get; set; }

        [Tooltip("Specifies the rate at which vertex data is advanced, such as per-vertex or per-instance.")]
        public VkVertexInputRate inputRate { get; set; }
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct VkVertexInputAttributeDescription
    {
        public uint location;
        public uint binding;
        public VkFormat format;
        public uint offset;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineVertexInputStateCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkCullModeFlagBits flags;
        public uint vertexBindingDescriptionCount;
        public VkVertexInputBindingDescription* pVertexBindingDescriptions;
        public uint vertexAttributeDescriptionCount;
        public VkVertexInputAttributeDescription* pVertexAttributeDescriptions;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineDynamicStateCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkPipelineColorBlendStateCreateFlagBits flags;
        public uint dynamicStateCount;
        public VkDynamicState* pDynamicStates;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkShaderModuleCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkImageViewCreateFlagBits flags;
        public size_t codeSize;
        public uint* pCode;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSpecializationMapEntry
    {
        public uint constantID;
        public uint offset;
        public size_t size;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSpecializationInfo
    {
        public uint mapEntryCount;
        public VkSpecializationMapEntry* pMapEntries;
        public size_t dataSize;
        public void* pData;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineShaderStageCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkPipelineShaderStageCreateFlagBits flags;
        public VkShaderStageFlagBits stage;
        public VkShaderModule module;
        public char* pName;
        public VkSpecializationInfo* pSpecializationInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineTessellationStateCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkCullModeFlagBits flags;
        public uint patchControlPoints;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkGraphicsPipelineCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkPipelineCreateFlagBits flags;
        public uint stageCount;
        public VkPipelineShaderStageCreateInfo* pStages;
        public VkPipelineVertexInputStateCreateInfo* pVertexInputState;
        public VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
        public VkPipelineTessellationStateCreateInfo* pTessellationState;
        public VkPipelineViewportStateCreateInfo* pViewportState;
        public VkPipelineRasterizationStateCreateInfo* pRasterizationState;
        public VkPipelineMultisampleStateCreateInfo* pMultisampleState;
        public VkPipelineDepthStencilStateCreateInfo* pDepthStencilState;
        public VkPipelineColorBlendStateCreateInfo* pColorBlendState;
        public VkPipelineDynamicStateCreateInfo* pDynamicState;
        public VkPipelineLayout layout;
        public VkRenderPass renderPass;
        public uint subpass;
        public VkPipeline basePipelineHandle;
        public int basePipelineIndex;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSemaphoreCreateInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkFenceCreateFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkBufferCopy
    {
        public ulong srcOffset;  // Byte offset into the source buffer
        public ulong dstOffset;  // Byte offset into the destination buffer
        public ulong size;       // Size in bytes to copy
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageCopy
    {
        public VkImageSubresourceLayers srcSubresource; // Source image subresource
        public VkOffset3D srcOffset;               // Offset in the source image
        public VkImageSubresourceLayers dstSubresource; // Destination image subresource
        public VkOffset3D dstOffset;               // Offset in the destination image
        public VkExtent3D extent;                   // Width, height, and depth of the region to copy
    }

    // Additional structs you may need
    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageSubresourceLayers
    {
        public VkImageAspectFlagBits aspectMask;     // Which aspects are being referenced
        public uint mipLevel;                      // Mip level of the image
        public uint baseArrayLayer;                // Base array layer for the copy operation
        public uint layerCount;                    // Number of layers for the copy operation
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkFormatProperties
    {
        public VkFormatFeatureFlagBits linearTilingFeatures;
        public VkFormatFeatureFlagBits optimalTilingFeatures;
        public VkFormatFeatureFlagBits bufferFeatures;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageBlit
    {
        public VkImageSubresourceLayers srcSubresource; // Source image subresource
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public VkOffset3D[] srcOffsets;                   // 3D coordinates offset for source image (2 offsets required)
        public VkImageSubresourceLayers dstSubresource;    // Destination image subresource
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public VkOffset3D[] dstOffsets;                   // 3D coordinates offset for destination image (2 offsets required)
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkMemoryAllocateFlagsInfo
    {
        public VkStructureType sType;
        public void* pNext;
        public VkMemoryAllocateFlagBits flags;
        public uint deviceMask;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkBufferImageCopy
    {
        public ulong bufferOffset;                // Byte offset into the buffer
        public uint bufferRowLength;              // Number of pixels per row in the buffer (0 means tightly packed)
        public uint bufferImageHeight;            // Number of rows in the buffer (0 means tightly packed)
        public VkImageSubresourceLayers imageSubresource; // Subresource details for the image
        public VkOffset3D imageOffset;            // Offset in the destination image
        public VkExtent3D imageExtent;            // Width, height, and depth of the image region
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageSubresourceRange
    {
        public VkImageAspectFlagBits aspectMask;    // Aspect(s) of the image to operate on
        public uint baseMipLevel;                 // Base mip level (level 0 is the highest resolution)
        public uint levelCount;                   // Number of mip levels (0 means all levels)
        public uint baseArrayLayer;               // Base array layer
        public uint layerCount;                   // Number of layers (0 means all layers)
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkClearAttachment
    {
        public VkImageAspectFlagBits aspectMask; // Aspect of the image to clear (e.g., color, depth)
        public uint colorAttachment;            // Index of the color attachment (if aspectMask has color bits)
        public VkClearValue clearValue;        // The value to clear to (color, depth, etc.)
    }

    // VkClearRect is used to specify a rectangle to clear in an image.
    [StructLayout(LayoutKind.Sequential)]
    public struct VkClearRect
    {
        public VkRect2D rect;                   // The rectangular area to clear
        public uint baseArrayLayer;              // Index of the first layer (useful for array layers)
        public uint layerCount;                  // Number of layers to clear
    }

    // VkImageResolve is used for resolving multisampled images.
    [StructLayout(LayoutKind.Sequential)]
    public struct VkImageResolve
    {
        public VkImageSubresourceLayers srcSubresource; // Source image subresource (multisampled)
        public VkOffset3D srcOffset;                    // Offset in the source image
        public VkImageSubresourceLayers dstSubresource; // Destination image subresource (single-sampled)
        public VkOffset3D dstOffset;                    // Offset in the destination image
        public VkExtent3D extent;                        // Width, height, and depth of the resolve region
    }

    // VkEvent is used for synchronization between command buffers.
    [StructLayout(LayoutKind.Sequential)]
    public struct VkEvent
    {
        // Note: In C#, this struct may work as a placeholder for the actual Vulkan event handle.
        // Since Vulkan events are not represented as structs, you would typically deal with them as IntPtr or similar.
        public IntPtr handle; // Placeholder for the Vulkan event handle
    }

    // VkMemoryBarrier is used to synchronize memory accesses.
    [StructLayout(LayoutKind.Sequential)]
    public struct VkMemoryBarrier
    {
        public VkAccessFlags srcAccessMask; // Access mask specifying which accesses occur before the barrier
        public VkAccessFlags dstAccessMask; // Access mask specifying which accesses occur after the barrier
    }

    // VkBufferMemoryBarrier is used to synchronize access to buffer resources.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkBufferMemoryBarrier
    {
        public VkStructureType sType;           // Structure type (Vulkan versioning)
        public void* pNext;                    // Pointer to extension-specific structure (if any)
        public VkAccessFlags srcAccessMask;     // Source access mask
        public VkAccessFlags dstAccessMask;     // Destination access mask
        public uint srcQueueFamilyIndex;        // Source queue family (0xFFFFFFFF for no sharing)
        public uint dstQueueFamilyIndex;        // Destination queue family (0xFFFFFFFF for no sharing)
        public VkBuffer buffer;                  // Buffer being accessed
        public ulong offset;                     // Offset in the buffer
        public ulong size;                       // Size of the buffer range (VK_WHOLE_SIZE for all)
    }

    // VkImageMemoryBarrier is used to synchronize access to image resources.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkImageMemoryBarrier
    {
        public VkStructureType sType;                  // Structure type (Vulkan versioning)
        public void* pNext;                           // Pointer to extension-specific structure (if any)
        public VkAccessFlagBits srcAccessMask;            // Source access mask
        public VkAccessFlagBits dstAccessMask;            // Destination access mask
        public VkImageLayout oldLayout;                // Old layout of the image
        public VkImageLayout newLayout;                // New layout of the image
        public uint srcQueueFamilyIndex;               // Source queue family (0xFFFFFFFF for no sharing)
        public uint dstQueueFamilyIndex;               // Destination queue family (0xFFFFFFFF for no sharing)
        public VkImage image;                           // Image being accessed
        public VkImageSubresourceRange subresourceRange; // Range of image subresources affected
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkSubmitInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures (NULL for none)
        public uint waitSemaphoreCount;                     // Number of wait semaphores
        public VkSemaphore* pWaitSemaphores;                     // Array of wait semaphores
        public VkPipelineStageFlagBits* pWaitDstStageMask;                   // Stage masks for the wait semaphores
        public uint commandBufferCount;                     // Number of command buffers to execute
        public VkCommandBuffer* pCommandBuffers;                      // Array of command buffers
        public uint signalSemaphoreCount;                   // Number of signal semaphores
        public VkSemaphore* pSignalSemaphores;                    // Array of signal semaphores
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPresentInfoKHR
    {
        public VkStructureType sType;
        public void* pNext;
        public uint waitSemaphoreCount;
        public VkSemaphore* pWaitSemaphores;
        public uint swapchainCount;
        public VkSwapchainKHR* pSwapchains;
        public uint* pImageIndices;
        public VkResult* pResults;
    }

    // VkMemoryAllocateInfo structure is used to allocate memory.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkMemoryAllocateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures (NULL for none)
        public ulong allocationSize;                         // Size of the allocation in bytes
        public uint memoryTypeIndex;                        // Index of the memory type from which to allocate
    }

    // VkMappedMemoryRange structure is used to define a range of memory that may be mapped.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkMappedMemoryRange
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures (NULL for none)
        public VkDeviceMemory memory;                                // Memory to be mapped
        public ulong offset;                                // Offset into the memory
        public ulong size;                                  // Size of the mapped range (VK_WHOLE_SIZE to map the whole memory)
    }

    // VkEventCreateInfo structure is used to create an event.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkEventCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures (NULL for none)
        public VkEventCreateFlagBits flags;                    // Event creation flags
    }

    // VkSubresourceLayout structure describes the layout of a subresource of an image.
    [StructLayout(LayoutKind.Sequential)]
    public struct VkSubresourceLayout
    {
        public ulong offset;                                // Offset in bytes from the start of the memory
        public ulong size;                                  // Size in bytes of the subresource
        public uint rowPitch;                               // Row pitch in bytes
        public uint arrayPitch;                             // Array pitch in bytes
        public uint depthPitch;                             // Depth pitch in bytes
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkCommandBufferAllocateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structure
        public VkCommandPool commandPool;                   // Command pool from which the command buffers are allocated
        public VkCommandBufferLevel level;                  // Level of command buffer (primary or secondary)
        public uint commandBufferCount;                     // Number of command buffers to allocate
    }

    // VkCommandPoolCreateInfo is used to create a command pool.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkCommandPoolCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structure
        public VkCommandPoolCreateFlagBits flags;              // Command pool creation flags
        public uint queueFamilyIndex;                        // Queue family index
    }

    // VkWriteDescriptorSet is used to update descriptor sets.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkWriteDescriptorSet
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structure
        public VkDescriptorSet dstSet;                      // Destination descriptor set
        public uint dstBinding;                             // Binding within the descriptor set
        public uint dstArrayElement;                        // Array element within the binding
        public uint descriptorCount;                        // Number of descriptors to update
        public VkDescriptorType descriptorType;            // Type of descriptors to update
        public VkDescriptorImageInfo* pImageInfo;                          // Pointer to image info (VkDescriptorImageInfo)
        public VkDescriptorBufferInfo* pBufferInfo;                         // Pointer to buffer info (VkDescriptorBufferInfo)
        public VkBufferView* pTexelBufferView;                    // Pointer to buffer views for texel buffers
    }

    // VkCopyDescriptorSet is used to copy descriptors from one set to another.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkCopyDescriptorSet
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structure
        public VkDescriptorSet srcSet;                      // Source descriptor set
        public uint srcBinding;                             // Binding within the source descriptor set
        public uint srcArrayElement;                        // Array element within the source binding
        public VkDescriptorSet dstSet;                      // Destination descriptor set
        public uint dstBinding;                             // Binding within the destination descriptor set
        public uint dstArrayElement;                        // Array element within the destination binding
        public uint descriptorCount;                        // Number of descriptors to copy
    }

    // VkDescriptorSetAllocateInfo is used to allocate descriptor sets.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkDescriptorSetAllocateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structure
        public VkDescriptorPool descriptorPool;             // Descriptor pool from which to allocate
        public uint descriptorSetCount;                     // Number of descriptor sets to allocate
        public VkDescriptorSet* pSetLayouts;                          // Pointer to array of descriptor set layouts
    }

    // VkDescriptorSetLayoutCreateInfo is used to create a descriptor set layout.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkDescriptorSetLayoutCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkDescriptorSetLayoutCreateFlagBits flags;      // Descriptor set layout creation flags
        public uint bindingCount;                           // Number of bindings in the layout
        public VkDescriptorSetLayoutBinding* pBindings;                            // Pointer to array of descriptor set layout bindings
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkPushConstantRange
    {
        public VkShaderStageFlagBits stageFlags;
        public uint offset;
        public uint size;
    }

    // VkPipelineLayoutCreateInfo is used to create a pipeline layout.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineLayoutCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkPipelineLayoutCreateFlagBits flags;           // Pipeline layout creation flags
        public uint setLayoutCount;                         // Number of descriptor set layouts
        public VkDescriptorSetLayout* pSetLayouts;                          // Pointer to array of descriptor set layouts
        public uint pushConstantRangeCount;                 // Number of push constant ranges
        public VkPushConstantRange* pPushConstantRanges;                  // Pointer to array of push constant ranges
    }

    // VkComputePipelineCreateInfo is used to create compute pipelines.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkComputePipelineCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkPipelineCreateFlagBits flags;                 // Pipeline creation flags
        public VkPipelineShaderStageCreateInfo stage;      // Shader stage for the compute pipeline
        public VkPipelineLayout layout;                     // Pipeline layout
        public VkPipeline basePipelineHandle;               // Handle to the base pipeline
        public int basePipelineIndex;                       // Index of the base pipeline
    }

    // VkPipelineCacheCreateInfo is used to create a pipeline cache.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkPipelineCacheCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkPipelineCacheCreateFlagBits flags;           // Pipeline cache creation flags
        public ulong initialSize;                           // Initial size of the pipeline cache
        public void* pInitialData;                         // Pointer to initial data for the pipeline cache
    }

    // VkImageViewCreateInfo is used to create image views.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkImageViewCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkImageViewCreateFlagBits flags;                // Image view creation flags
        public VkImage image;                               // Image to create a view for
        public VkImageViewType viewType;                   // Type of image view (1D, 2D, 3D, etc.)
        public VkFormat format;                             // Format of the image data
        public VkComponentMapping components;              // Component mapping for the image view
        public VkImageSubresourceRange subresourceRange;   // Subresource range for the image view
    }

    // VkBufferViewCreateInfo is used to create buffer views.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkBufferViewCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkBufferUsageFlagBits flags;              // Buffer view creation flags
        public VkBuffer buffer;                             // Buffer to create a view for
        public VkFormat format;                             // Format for the buffer view
        public ulong offset;                                // Offset within the buffer
        public ulong range;                                 // Size of the buffer view range
    }

    // VkBufferCreateInfo is used to create buffer objects.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkBufferCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkBufferCreateFlagBits flags;                   // Buffer creation flags
        public ulong size;                                  // Size of the buffer
        public VkBufferUsageFlagBits usage;                   // Buffer usage flags
        public VkSharingMode sharingMode;
        public uint queueFamilyIndexCount;                  // Number of queue families
        public uint pQueueFamilyIndices;                  // Pointer to array of queue family indices
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkSurfaceFormatKHR
    {
        public VkFormat format;
        public VkColorSpaceKHR colorSpace;
    }

    // VkQueryPoolCreateInfo is used to create query pools.
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VkQueryPoolCreateInfo
    {
        public VkStructureType sType;                       // Structure type
        public void* pNext;                                // Pointer to extension-specific structures
        public VkQueryPipelineStatisticFlagBits flags;                // Query pool creation flags
        public VkQueryType queryType;                       // Type of queries (occlusion, timestamp, etc.)
        public uint queryCount;                             // Number of queries
        public uint pipelineStatistics;                      // Pipeline statistics to query
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkComponentMapping
    {
        public VkComponentSwizzle r;                       // Swizzle component for red
        public VkComponentSwizzle g;                       // Swizzle component for green
        public VkComponentSwizzle b;                       // Swizzle component for blue
        public VkComponentSwizzle a;                       // Swizzle component for alpha
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkDescriptorImageInfo
    {
        public VkSampler sampler;       // A handle to a sampler object
        public VkImageView imageView;     // A handle to an image view
        public VkImageLayout imageLayout; // The layout the image is in for access by shaders

        public VkDescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
        {
            this.sampler = sampler;
            this.imageView = imageView;
            this.imageLayout = imageLayout;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkDescriptorBufferInfo
    {
        public VkBuffer buffer;      // A handle to the buffer
        public ulong offset;       // Offset in the buffer where the data begins
        public ulong range;        // Size of the buffer data referred to by the descriptor (can be VK_WHOLE_SIZE)

        public VkDescriptorBufferInfo(VkBuffer buffer, ulong offset, ulong range)
        {
            this.buffer = buffer;
            this.offset = offset;
            this.range = range;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkPhysicalDeviceMemoryProperties
    {
        public uint memoryTypeCount;
        public VkMemoryType[] memoryTypes = new VkMemoryType[VulkanCSConst.VK_MAX_MEMORY_TYPES];
        public uint memoryHeapCount;
        public VkMemoryHeap[] memoryHeaps = new VkMemoryHeap[VulkanCSConst.VK_MAX_MEMORY_HEAPS];

        public VkPhysicalDeviceMemoryProperties()
        {
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkMemoryHeap
    {
        public VkDeviceSize size;
        public VkMemoryHeapFlagBits flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VkMemoryType
    {
        public VkMemoryPropertyFlagBits propertyFlags;
        public uint heapIndex;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public struct VkPhysicalDeviceFeatures
    {
        VkBool32 robustBufferAccess;
        VkBool32 fullDrawIndexUint32;
        VkBool32 imageCubeArray;
        VkBool32 independentBlend;
        VkBool32 geometryShader;
        VkBool32 tessellationShader;
        VkBool32 sampleRateShading;
        VkBool32 dualSrcBlend;
        VkBool32 logicOp;
        VkBool32 multiDrawIndirect;
        VkBool32 drawIndirectFirstInstance;
        VkBool32 depthClamp;
        VkBool32 depthBiasClamp;
        VkBool32 fillModeNonSolid;
        VkBool32 depthBounds;
        VkBool32 wideLines;
        VkBool32 largePoints;
        VkBool32 alphaToOne;
        VkBool32 multiViewport;
        VkBool32 samplerAnisotropy;
        VkBool32 textureCompressionETC2;
        VkBool32 textureCompressionASTC_LDR;
        VkBool32 textureCompressionBC;
        VkBool32 occlusionQueryPrecise;
        VkBool32 pipelineStatisticsQuery;
        VkBool32 vertexPipelineStoresAndAtomics;
        VkBool32 fragmentStoresAndAtomics;
        VkBool32 shaderTessellationAndGeometryPointSize;
        VkBool32 shaderImageGatherExtended;
        VkBool32 shaderStorageImageExtendedFormats;
        VkBool32 shaderStorageImageMultisample;
        VkBool32 shaderStorageImageReadWithoutFormat;
        VkBool32 shaderStorageImageWriteWithoutFormat;
        VkBool32 shaderUniformBufferArrayDynamicIndexing;
        VkBool32 shaderSampledImageArrayDynamicIndexing;
        VkBool32 shaderStorageBufferArrayDynamicIndexing;
        VkBool32 shaderStorageImageArrayDynamicIndexing;
        VkBool32 shaderClipDistance;
        VkBool32 shaderCullDistance;
        VkBool32 shaderFloat64;
        VkBool32 shaderInt64;
        VkBool32 shaderInt16;
        VkBool32 shaderResourceResidency;
        VkBool32 shaderResourceMinLod;
        VkBool32 sparseBinding;
        VkBool32 sparseResidencyBuffer;
        VkBool32 sparseResidencyImage2D;
        VkBool32 sparseResidencyImage3D;
        VkBool32 sparseResidency2Samples;
        VkBool32 sparseResidency4Samples;
        VkBool32 sparseResidency8Samples;
        VkBool32 sparseResidency16Samples;
        VkBool32 sparseResidencyAliased;
        VkBool32 variableMultisampleRate;
        VkBool32 inheritedQueries;
    }
}