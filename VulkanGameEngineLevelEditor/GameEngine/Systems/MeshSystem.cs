using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public enum MeshTypeEnum
    {
        Mesh_SpriteMesh,
        Mesh_LevelMesh,
        Mesh_LineMesh
    };

    public unsafe static class MeshSystem
    {
        public static uint CreateMesh(MeshTypeEnum meshType, ListPtr<Vertex2D> vertexList, ListPtr<UInt32> indexList)
        {
            return DLLSystem.CallDLLFunc(() => MeshSystem_CreateMesh(meshType, vertexList.Ptr, indexList.Ptr, vertexList.Count, indexList.Count));
        }
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Update(deltaTime));
        }
        public static void Destroy(uint meshId)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Destroy(meshId));
        }
        public static void DestroyMesh(Mesh mesh, VulkanBuffer vertexBuffer, VulkanBuffer indexBuffer, VulkanBuffer transformBuffer, VulkanBuffer propertiesBuffer)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_DestroyMesh(mesh, vertexBuffer, indexBuffer, transformBuffer, propertiesBuffer));
        }
        public static void DestroyAllGameObjects()
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_DestroyAllGameObjects());
        }
        public static Mesh FindMesh(uint meshId)
        {
            return DLLSystem.CallDLLFunc(() => MeshSystem_FindMesh(meshId));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint MeshSystem_CreateMesh(MeshTypeEnum meshType, Vertex2D* vertexListPtr, UInt32* indexListPtr, size_t vertexListCount, size_t indexListCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_Update(float deltaTime);
		[DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_Destroy(uint meshId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_DestroyMesh(Mesh mesh, VulkanBuffer vertexBuffer, VulkanBuffer indexBuffer, VulkanBuffer transformBuffer, VulkanBuffer propertiesBuffer);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_DestroyAllGameObjects();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Mesh MeshSystem_FindMesh(uint meshId);
    }
}
