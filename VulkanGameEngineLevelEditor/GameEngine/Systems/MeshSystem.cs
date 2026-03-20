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

    public enum VertexLayoutEnum
    {
        kVertexLayout_NullVertex,
        kVertexLayout_Vertex2D,
        kVertexLayout_SpriteInstanceVertex,
        kVertexLayout_SkyBoxVertex,
        kVertexLayout_Undefined
    };

    public unsafe struct VertexLayout
    {
        public VertexLayoutEnum VertexType { get; set; } = VertexLayoutEnum.kVertexLayout_Undefined;
        public UInt64 VertexDataSize { get; set; } = UInt64.MaxValue;
        public void* VertexData { get; set; } = null;

        public VertexLayout()
        {
        }
    };

    public unsafe static class MeshSystem
    {
        public static uint CreateMesh(string key, MeshTypeEnum meshType, VertexLayout vertexData, ListPtr<uint> indexList, Guid materialId = new Guid())
        {
            return DLLSystem.CallDLLFunc(() => MeshSystem_CreateMesh(key, meshType, vertexData, indexList.Ptr, indexList.Count, materialId));
        }
        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Update(deltaTime));
        }
        public static void Destroy(uint meshId)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Destroy(meshId));
        }
        public static void DestroyMesh(uint meshId)
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_Destroy(meshId));
        }

        public static void DestroyAllGameObjects()
        {
            DLLSystem.CallDLLFunc(() => MeshSystem_DestroyAllGameObjects());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint MeshSystem_CreateMesh([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string key, MeshTypeEnum meshType, VertexLayout vertexData, uint* indexListPtr, size_t indexListCount, Guid materialId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_Update(float deltaTime);
	    [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_Destroy(uint meshId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void MeshSystem_DestroyAllGameObjects();
    }
}
