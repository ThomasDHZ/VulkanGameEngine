using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
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
            MaterialMap[materialJson.MaterialId] = Material_CreateMaterial(ref renderer, NextBufferIndex, out VulkanBuffer buffer, materialPath);
            BufferSystem.VulkanBufferMap[NextBufferIndex] = buffer;

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
                MaterialProperitiesBuffer materialBufferProperties = new MaterialProperitiesBuffer
                {
                    AlbedoMapId = material.AlbedoMapId != new Guid() ? TextureSystem.TextureList[material.AlbedoMapId].textureBufferIndex : 0,
                    MetallicRoughnessMapId = material.MetallicRoughnessMapId != new Guid() ? TextureSystem.TextureList[material.MetallicRoughnessMapId].textureBufferIndex : 0,
                    MetallicMapId = material.MetallicMapId != new Guid() ? TextureSystem.TextureList[material.MetallicMapId].textureBufferIndex : 0,
                    RoughnessMapId = material.RoughnessMapId != new Guid() ? TextureSystem.TextureList[material.RoughnessMapId].textureBufferIndex : 0,
                    AmbientOcclusionMapId = material.AmbientOcclusionMapId != new Guid() ? TextureSystem.TextureList[material.AmbientOcclusionMapId].textureBufferIndex : 0,
                    NormalMapId = material.NormalMapId != new Guid() ? TextureSystem.TextureList[material.NormalMapId].textureBufferIndex : 0,
                    DepthMapId = material.DepthMapId != new Guid() ? TextureSystem.TextureList[material.DepthMapId].textureBufferIndex : 0,
                    AlphaMapId = material.AlphaMapId != new Guid() ? TextureSystem.TextureList[material.AlphaMapId].textureBufferIndex : 0,
                    EmissionMapId = material.EmissionMapId != new Guid() ? TextureSystem.TextureList[material.EmissionMapId].textureBufferIndex : 0,
                    HeightMapId = material.HeightMapId != new Guid() ? TextureSystem.TextureList[material.HeightMapId].textureBufferIndex : 0
                };

                Material_UpdateBuffer(RenderSystem.renderer, BufferSystem.VulkanBufferMap[(uint)material.MaterialBufferId], materialBufferProperties);
                x++;
            }
        }

        public static Material FindMaterial(Guid renderPassGuid)
        {
            return MaterialMap.Where(x => x.Key == renderPassGuid).First().Value;
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern Material Material_CreateMaterial(ref GraphicsRenderer renderer, uint bufferIndex, out VulkanBuffer materialBuffer, [MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string jsonString);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Material_UpdateBuffer(GraphicsRenderer renderer, VulkanBuffer materialBuffer, MaterialProperitiesBuffer materialProperties);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Material_DestroyBuffer(GraphicsRenderer renderer, VulkanBuffer materialBuffer);
    }
}
