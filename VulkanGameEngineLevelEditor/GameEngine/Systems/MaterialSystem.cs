using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Silk.NET.Core.Native;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{

    public static unsafe class MaterialSystem
    {
        private static uint NextBufferID = 0;
        public static Dictionary<Guid, Material> MaterialMap { get; private set; } = new Dictionary<Guid, Material>();

        public static Guid LoadMaterial(string materialPath)
        {
            if (materialPath.IsEmpty())
            {
                return new Guid();
            }

            string jsonContent = File.ReadAllText(materialPath);
            Material materialJson = JsonConvert.DeserializeObject<Material>(jsonContent);

            if (MaterialMap.ContainsKey(materialJson.MaterialId))
            {
                return materialJson.MaterialId;
            }

            GraphicsRenderer renderer = RenderSystem.renderer;
            uint NextBufferIndex = ++BufferSystem.NextBufferId;
            ShaderSystem.PipelineShaderStructMap[(int)NextBufferIndex] = ShaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer", (int)NextBufferIndex);
            MaterialMap[materialJson.MaterialId] = Material_CreateMaterial(ref renderer, NextBufferIndex, out VulkanBuffer vulkanBuffer, ShaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer", (int)NextBufferIndex), materialPath);
            BufferSystem.VulkanBufferMap[NextBufferIndex] = vulkanBuffer;

            return materialJson.MaterialId;
        }

        public static ListPtr<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer()
        {
            ListPtr<VkDescriptorBufferInfo> materialPropertiesBuffer = new ListPtr<VkDescriptorBufferInfo>();
            if (MaterialMap.Any())
            {
                materialPropertiesBuffer.Add(new VkDescriptorBufferInfo
                {
                    buffer = VulkanCSConst.VK_NULL_HANDLE,
                    offset = 0,
                    range = VulkanCSConst.VK_WHOLE_SIZE
                });
            }
            else
            {
                foreach (var material in MaterialMap)
                {
                    VkDescriptorBufferInfo meshBufferInfo = new VkDescriptorBufferInfo
                    {
                        buffer = BufferSystem.VulkanBufferMap[(uint)material.Value.MaterialBufferId].Buffer,
                        offset = 0,
                        range = VulkanCSConst.VK_WHOLE_SIZE
                    };
                    materialPropertiesBuffer.Add(meshBufferInfo);
                }
            }
            return materialPropertiesBuffer;
        }

        public static void Update(float deltaTime)
        {
            uint x = 0;
            foreach (var materialPair in MaterialMap)
            {
                Material material = materialPair.Value;
                uint AlbedoMapId = material.AlbedoMapId != Guid.Empty ? TextureSystem.FindTexture(material.AlbedoMapId).textureBufferIndex : 0;
                uint MetallicRoughnessMapId = material.MetallicRoughnessMapId != Guid.Empty ? TextureSystem.FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : 0;
                uint MetallicMapId = material.MetallicMapId != Guid.Empty ? TextureSystem.FindTexture(material.MetallicMapId).textureBufferIndex : 0;
                uint RoughnessMapId = material.RoughnessMapId != Guid.Empty ? TextureSystem.FindTexture(material.RoughnessMapId).textureBufferIndex : 0;
                uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != Guid.Empty ? TextureSystem.FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : 0;
                uint NormalMapId = material.NormalMapId != Guid.Empty ? TextureSystem.FindTexture(material.NormalMapId).textureBufferIndex : 0;
                uint DepthMapId = material.DepthMapId != Guid.Empty ? TextureSystem.FindTexture(material.DepthMapId).textureBufferIndex : 0;
                uint AlphaMapId = material.AlphaMapId != Guid.Empty ? TextureSystem.FindTexture(material.AlphaMapId).textureBufferIndex : 0;
                uint EmissionMapId = material.EmissionMapId != Guid.Empty ? TextureSystem.FindTexture(material.EmissionMapId).textureBufferIndex : 0;
                uint HeightMapId = material.HeightMapId != Guid.Empty ? TextureSystem.FindTexture(material.HeightMapId).textureBufferIndex : 0;


                ShaderStruct shaderStruct = ShaderSystem.FindShaderStruct(material.MaterialBufferId);
                var shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "AlbedoMap");
                if (shaderVar != null &&
                    (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = AlbedoMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "MetallicRoughnessMap");
                if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = MetallicRoughnessMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "MetallicMap");
                if (shaderVar != null &&
                    (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = MetallicMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "RoughnessMapMap");
                if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = RoughnessMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "AmbientOcclusionMap");
                if (shaderVar != null &&
                    (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = AmbientOcclusionMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "NormalMap");
                if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = NormalMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "DepthMap");
                if (shaderVar != null &&
                    (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = DepthMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "AlphaMap");
                if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = AlphaMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "EmissionMap");
                if (shaderVar != null &&
                    (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = EmissionMapId;
                }

                shaderVar = ShaderSystem.SearchShaderStruct(shaderStruct, "HeightMap");
                if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
                {
                    *(uint*)shaderVar->Value = HeightMapId;
                }
                ShaderSystem.UpdateShaderBuffer(material.MaterialBufferId);
                x++;
            }
        }

        public static Material FindMaterial(Guid renderPassGuid)
        {
            return MaterialMap.Where(x => x.Key == renderPassGuid).First().Value;
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Material Material_CreateMaterial(ref GraphicsRenderer renderer, uint bufferIndex, out VulkanBuffer vulkanBuffer, ShaderStruct shaderStruct, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Material_UpdateBuffer(GraphicsRenderer renderer, VulkanBuffer materialBuffer, MaterialProperitiesBuffer materialProperties);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Material_DestroyBuffer(GraphicsRenderer renderer, VulkanBuffer materialBuffer);
    }
}
