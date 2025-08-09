using Silk.NET.Vulkan;
using System;
using System.Runtime.InteropServices;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    public enum DescriptorBindingPropertiesEnum : UInt32
    {
        kMeshPropertiesDescriptor,
        kTextureDescriptor,
        kMaterialDescriptor,
        kBRDFMapDescriptor,
        kIrradianceMapDescriptor,
        kPrefilterMapDescriptor,
        kCubeMapDescriptor,
        kEnvironmentDescriptor,
        kSunLightDescriptor,
        kDirectionalLightDescriptor,
        kPointLightDescriptor,
        kSpotLightDescriptor,
        kReflectionViewDescriptor,
        kDirectionalShadowDescriptor,
        kPointShadowDescriptor,
        kSpotShadowDescriptor,
        kViewTextureDescriptor,
        kViewDepthTextureDescriptor,
        kCubeMapSamplerDescriptor,
        kRotatingPaletteTextureDescriptor,
        kMathOpperation1Descriptor,
        kMathOpperation2Descriptor,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct PipelineDescriptorModel
    {
        public PipelineDescriptorModel()
        {
        }

        [Tooltip("Specifies the binding number for the pipeline descriptor.")]
        public uint BindingNumber { get; set; }
        public uint DstArrayElement { get; set; }
        [Tooltip("Defines the binding properties for the descriptor.")]
        public DescriptorBindingPropertiesEnum BindingPropertiesList { get; set; }

        [Tooltip("Specifies the Vulkan descriptor type for this binding.")]
        public VkDescriptorType DescriptorType { get; set; }
        public VkShaderStageFlagBits StageFlags { get; set; }
         IntPtr pTextureSampler { get; set; } = IntPtr.Zero;
         IntPtr pTexelBufferView { get; set; } = IntPtr.Zero;
    }
}
