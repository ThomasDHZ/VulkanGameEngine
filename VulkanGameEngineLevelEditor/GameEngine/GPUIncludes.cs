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
    public unsafe class GPUIncludes
    {
        public size_t VertexPropertiesCount { get; set; } = 0;
        public size_t IndexPropertiesCount { get; set; } = 0;
        public size_t TransformPropertiesCount { get; set; } = 0;
        public size_t MeshPropertiesCount { get; set; } = 0;
        public size_t LevelLayerMeshPropertiesCount { get; set; } = 0;
        public size_t TexturePropertiesCount { get; set; } = 0;
        public size_t MaterialPropertiesCount { get; set; } = 0;
        public VkDescriptorBufferInfo* VertexProperties { get; set; } = null;
        public VkDescriptorBufferInfo* IndexProperties { get; set; } = null;
        public VkDescriptorBufferInfo* TransformProperties { get; set; } = null;
        public VkDescriptorBufferInfo* MeshProperties { get; set; } = null;
        public VkDescriptorBufferInfo* LevelLayerMeshProperties { get; set; } = null;
        public VkDescriptorImageInfo* TextureProperties { get; set; } = null;
        public VkDescriptorBufferInfo* MaterialProperties { get; set; } = null;
    };
}

