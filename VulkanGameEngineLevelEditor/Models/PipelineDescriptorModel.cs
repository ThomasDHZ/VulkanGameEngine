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
        [Tooltip("Specifies the binding number for the pipeline descriptor.")]
        public uint BindingNumber { get; set; }
        public uint DstArrayElement { get; set; }
        [Tooltip("Defines the binding properties for the descriptor.")]
        public DescriptorBindingPropertiesEnum BindingPropertiesList { get; set; }

        [Tooltip("Specifies the Vulkan descriptor type for this binding.")]
        public VkDescriptorType DescriptorType { get; set; }
        VkShaderStageFlagBits StageFlags { get; set; }
        VkSampler* pTextureSampler { get; set; }
        VkBufferView* pTexelBufferView { get; set; }

        public PipelineDescriptorModel(uint bindingNumber, DescriptorBindingPropertiesEnum properties, VkDescriptorType type)
        {
            BindingNumber = bindingNumber;
            BindingPropertiesList = properties;
            DescriptorType = type;
        }
    }
}
