using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct GPUIncludes
    {
        public size_t VertexPropertiesCount { get; set; }
        public size_t IndexPropertiesCount { get; set; }
        public size_t TransformPropertiesCount { get; set; }
        public size_t MeshPropertiesCount { get; set; }
        public size_t TexturePropertiesCount { get; set; }
        public size_t MaterialPropertiesCount { get; set; }
        public VkDescriptorBufferInfo* VertexProperties { get; set; }
        public VkDescriptorBufferInfo* IndexProperties { get; set; }
        public VkDescriptorBufferInfo* TransformProperties { get; set; }
        public VkDescriptorBufferInfo* MeshProperties { get; set; }
        public VkDescriptorImageInfo* TextureProperties { get; set; }
        public VkDescriptorBufferInfo* MaterialProperties { get; set; }
    }
}

