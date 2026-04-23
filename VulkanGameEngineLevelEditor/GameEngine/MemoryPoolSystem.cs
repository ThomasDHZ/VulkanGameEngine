using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public enum MemoryPoolTypes
    {
        kMeshBuffer,
        kMaterialBuffer,
        kDirectionalLightBuffer,
        kPointLightBuffer,
        kTexture2DMetadataBuffer,
        kTexture3DMetadataBuffer,
        kTextureCubeMapMetadataBuffer,
        kSpriteInstanceBuffer,
        kEndofPool
    };

    public struct DirectionalLight
    {
        [NumericUpDownLimitsAttribute(0.01f, 0.0f, 1.0f)]
        public vec3 LightColor { get; set; } = new vec3(1.0f, 1.0f, 1.0f);
        public vec3 LightDirection = new vec3(0.3f, 0.3f, 1.0f);
        public float LightIntensity = 1.5f;
        public float ShadowStrength = 1.0f;
        public float ShadowBias = 0.012f;
        public float ShadowSoftness = 0.008f;

        public DirectionalLight()
        {
        }
    };

    public struct PointLight
    {
        [NumericUpDownLimitsAttribute(10.0f)]
        public vec3 LightPosition;
        [NumericUpDownLimitsAttribute(0.01f, 0.0f, 1.0f)]
        public vec3 LightColor = new vec3(1.0f, 0.95f, 0.8f);
        public float LightRadius = 200.0f;
        public float LightIntensity = 2.0f;
        public float ShadowStrength = 1.0f;
        public float ShadowBias = 0.012f;
        public float ShadowSoftness = 0.008f;

        public PointLight()
        {
        }
    };

    public struct SceneDataBuffer
    {
        public uint BRDFMapId { get; set; }
        public uint CubeMapId { get; set; }
        public uint IrradianceMapId { get; set; }
        public uint PrefilterMapId { get; set; }
        public mat4 Projection { get; set; }
        public mat4 View { get; set; }
        public mat4 InverseProjection { get; set; }
        public mat4 InverseView { get; set; }
        public vec3 CameraPosition { get; set; }
        public float _pad0 { get; set; }
        public vec3 ViewDirection { get; set; }
        public float _pad1 { get; set; }
        public vec2 InvertResolution { get; set; }
        public float Time { get; set; }
        public uint FrameIndex { get; set; }
    };

    public struct MeshPropertiesStruct
    {
        public uint MaterialIndex { get; set; }
        public mat4 MeshTransform { get; set; }
    };

    public struct GPUMaterial
    {
        public uint AlbedoDataId { get; set; }
        public uint NormalDataId { get; set; }
        public uint PackedMRODataId { get; set; }
        public uint PackedSheenSSSDataId { get; set; }
        public uint UnusedDataId { get; set; }
        public uint EmissionDataId { get; set; }
    };

    public static unsafe class MemoryPoolSystem
    {
        public static void StartUp()
        {
             DLLSystem.CallDLLFunc(() => MemoryPoolSystem_StartUp());
        }

        public static uint AllocateObject(MemoryPoolTypes memoryPoolToUpdate)
        {
            return DLLSystem.CallDLLFunc(() => MemoryPoolSystem_AllocateObject(memoryPoolToUpdate));
        }

        public static MeshPropertiesStruct* UpdateMesh(uint index)
        {
            try
            {
                return MemoryPoolSystem_UpdateMesh(index);
            }
            catch (Exception ex) 
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static GPUMaterial* UpdateMaterial(uint index)
        {
            try
            {
                return MemoryPoolSystem_UpdateMaterial(index);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static DirectionalLight* UpdateDirectionalLight(uint index)
        {
            try
            {
                return MemoryPoolSystem_UpdateDirectionalLight(index);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static PointLight* UpdatePointLight(uint index)
        {
            try
            {
                return MemoryPoolSystem_UpdatePointLight(index);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static SceneDataBuffer* UpdateSceneDataBuffer()
        {
            try
            {
                return MemoryPoolSystem_UpdateSceneDataBuffer();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                MessageBox.Show(ex.ToString());
                return null;
            }
        }

        public static void FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint index)
        {
            DLLSystem.CallDLLFunc(() => MemoryPoolSystem_FreeObject(memoryPoolToUpdate, index));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MemoryPoolSystem_StartUp();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint MemoryPoolSystem_AllocateObject(MemoryPoolTypes memoryPoolToUpdate);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern MeshPropertiesStruct* MemoryPoolSystem_UpdateMesh(uint index);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GPUMaterial* MemoryPoolSystem_UpdateMaterial(uint index);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern DirectionalLight* MemoryPoolSystem_UpdateDirectionalLight(uint index);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern PointLight* MemoryPoolSystem_UpdatePointLight(uint index);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern SceneDataBuffer* MemoryPoolSystem_UpdateSceneDataBuffer();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MemoryPoolSystem_FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint index);
    }
}
