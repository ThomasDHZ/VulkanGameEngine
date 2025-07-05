﻿using GlmSharp;
using Silk.NET.SDL;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor;
using VulkanGameEngineLevelEditor.GameEngineAPI;

namespace VulkanGameEngineLevelEditor.Systems
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VertexLoaderStruct
    {
        public BufferTypeEnum VertexType { get; set; }
        public uint MeshVertexBufferId { get; set; }
        public size_t SizeofVertex { get; set; }
        public size_t VertexCount { get; set; }
        public void* VertexData { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct IndexLoaderStruct
    {
        public uint MeshIndexBufferId { get; set; }
        public size_t SizeofIndex { get; set; }
        public size_t IndexCount { get; set; }
        public void* IndexData { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct TransformLoaderStruct
    {
        public uint MeshTransformBufferId { get; set; }
        public size_t SizeofTransform { get; set; }
        public void* TransformData { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct MeshPropertiesLoaderStruct
    {
        public uint PropertiesBufferId { get; set; }
        public size_t SizeofMeshProperties { get; set; }
        public void* MeshPropertiesData { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct MeshLoader
    {
        public uint ParentGameObjectID { get; set; }
        public uint MeshId { get; set; }
        public Guid MaterialId { get; set; }

        public VertexLoaderStruct VertexLoader { get; set; }
        public IndexLoaderStruct IndexLoader { get; set; }
        public TransformLoaderStruct TransformLoader { get; set; }
        public MeshPropertiesLoaderStruct MeshPropertiesLoader { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 16)]
    public struct MeshPropertiesStruct
    {
        public uint ShaderMaterialBufferIndex = 0;
        public mat4 MeshTransform = mat4.Identity;

        public MeshPropertiesStruct()
        {
        }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 0)] // Try default alignment; revert to Pack = 16 if confirmed
    public struct Mesh
    {
        public uint MeshId;
        public uint ParentGameObjectID;
        public uint GameObjectTransform;
        public nuint VertexCount; // Match size_t (8 bytes on 64-bit)
        public nuint IndexCount;  // Match size_t
        public Guid MaterialId;   // Assuming VkGuid is a standard GUID
        public BufferTypeEnum VertexType;
        public vec3 MeshPosition;
        public vec3 MeshRotation;
        public vec3 MeshScale;
        public int MeshVertexBufferId;
        public int MeshIndexBufferId;
        public int MeshTransformBufferId;
        public int PropertiesBufferId;
        public MeshPropertiesStruct MeshProperties;
    };

    public unsafe static class MeshSystem
    {
        public static uint NextMeshId { get; private set; } = 0;
        public static uint NextSpriteMeshId { get; private set; } = 0;
        public static uint NextLevelLayerMeshId { get; private set; } = 0;

        public static Dictionary<uint, Mesh> MeshMap { get; private set; } = new Dictionary<uint, Mesh>();
        public static Dictionary<int, Mesh> SpriteMeshMap { get; private set; } = new Dictionary<int, Mesh>();
        public static Dictionary<Guid, ListPtr<Mesh>> LevelLayerMeshListMap { get; private set; } = new Dictionary<Guid, ListPtr<Mesh>>();
        public static Dictionary<uint, ListPtr<Vertex2D>> Vertex2DListMap { get; private set; } = new Dictionary<uint, ListPtr<Vertex2D>>();
        public static Dictionary<uint, ListPtr<uint>> IndexListMap { get; private set; } = new Dictionary<uint, ListPtr<uint>>();

        public static uint CreateMesh<T>(ListPtr<T> vertexList, ListPtr<uint> indexList, Guid materialId) where T : unmanaged
        {
            if (typeof(T) != typeof(Vertex2D))
            {
                throw new ArgumentException("Vertex type must be Vertex2D", nameof(vertexList));
            }

            if (vertexList == null || vertexList.Ptr == null || indexList == null || indexList.Ptr == null)
            {
                throw new ArgumentNullException("Vertex or index list is null or disposed");
            }

            uint meshId = NextMeshId++;
            mat4 meshMatrix = mat4.Identity;

            ListPtr<Vertex2D> vertex2DList = vertexList as ListPtr<Vertex2D> ?? throw new InvalidCastException("Failed to cast vertexList to ListPtr<Vertex2D>");
            Vertex2DListMap[meshId] = vertex2DList;
            IndexListMap[meshId] = indexList;

            Mesh mesh = new Mesh();
            MeshMap[meshId] = mesh;

            GCHandle matrixHandle = GCHandle.Alloc(meshMatrix, GCHandleType.Pinned);
            GCHandle propertiesHandle = GCHandle.Alloc(mesh.MeshProperties, GCHandleType.Pinned);
            try
            {
                MeshLoader meshLoader = new MeshLoader
                {
                    ParentGameObjectID = 0,
                    MeshId = meshId,
                    MaterialId = materialId,
                    VertexLoader = new VertexLoaderStruct
                    {
                        VertexType = BufferTypeEnum.BufferType_Vector2D,
                        MeshVertexBufferId = ++BufferSystem.NextBufferId,
                        SizeofVertex = sizeof(T),
                        VertexCount = vertexList.Count,
                        VertexData = vertexList.Ptr
                    },
                    IndexLoader = new IndexLoaderStruct
                    {
                        MeshIndexBufferId = ++BufferSystem.NextBufferId,
                        SizeofIndex = sizeof(uint),
                        IndexCount = indexList.Count,
                        IndexData = indexList.Ptr
                    },
                    TransformLoader = new TransformLoaderStruct
                    {
                        MeshTransformBufferId = ++BufferSystem.NextBufferId,
                        SizeofTransform = sizeof(mat4),
                        TransformData = (void*)matrixHandle.AddrOfPinnedObject()
                    },
                    MeshPropertiesLoader = new MeshPropertiesLoaderStruct
                    {
                        PropertiesBufferId = ++BufferSystem.NextBufferId,
                        SizeofMeshProperties = sizeof(MeshPropertiesStruct),
                        MeshPropertiesData = (void*)propertiesHandle.AddrOfPinnedObject()
                    }
                };

                mesh = Mesh_CreateMesh(RenderSystem.renderer, meshLoader, out VulkanBuffer vertexBuffer, out VulkanBuffer indexBuffer, out VulkanBuffer meshTransformBuffer, out VulkanBuffer propertiesBuffer);
                BufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId] = vertexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId] = indexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId] = meshTransformBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = propertiesBuffer;
                MeshMap[meshId] = mesh;
            }
            finally
            {
                matrixHandle.Free();
                propertiesHandle.Free();
            }

            return meshId;
        }

        public static int CreateSpriteLayerMesh(ListPtr<Vertex2D> vertexList, ListPtr<uint> indexList)
        {
            uint meshId = ++NextSpriteMeshId;
            mat4 meshMatrix = mat4.Identity;

            Vertex2DListMap[meshId] = vertexList;
            IndexListMap[meshId] = indexList;

            Mesh mesh = new Mesh();
            MeshMap[meshId] = mesh;

            GCHandle matrixHandle = GCHandle.Alloc(meshMatrix, GCHandleType.Pinned);
            GCHandle propertiesHandle = GCHandle.Alloc(mesh.MeshProperties, GCHandleType.Pinned);
            try
            {
                MeshLoader meshLoader = new MeshLoader
                {
                    ParentGameObjectID = 0,
                    MeshId = meshId,
                    MaterialId = Guid.Empty,
                    VertexLoader = new VertexLoaderStruct

                    {
                        VertexType = BufferTypeEnum.BufferType_Vector2D,
                        MeshVertexBufferId = ++BufferSystem.NextBufferId,
                        SizeofVertex = sizeof(Vertex2D),
                        VertexCount = vertexList.Count,
                        VertexData = vertexList.Ptr,
                    },
                    IndexLoader = new IndexLoaderStruct

                    {
                        MeshIndexBufferId = ++BufferSystem.NextBufferId,
                        SizeofIndex = sizeof(uint),
                        IndexCount = indexList.Count,
                        IndexData = indexList.Ptr,
                    },
                    TransformLoader = new TransformLoaderStruct

                    {
                        MeshTransformBufferId = ++BufferSystem.NextBufferId,
                        SizeofTransform = sizeof(mat4),
                        TransformData = (void*)matrixHandle.AddrOfPinnedObject()
                    },
                    MeshPropertiesLoader = new MeshPropertiesLoaderStruct

                    {
                        PropertiesBufferId = ++BufferSystem.NextBufferId,
                        SizeofMeshProperties = sizeof(MeshPropertiesStruct),
                        MeshPropertiesData = (void*)propertiesHandle.AddrOfPinnedObject()
                    }
                };

                mesh = Mesh_CreateMesh(RenderSystem.renderer, meshLoader, out VulkanBuffer vertexBuffer, out VulkanBuffer indexBuffer, out VulkanBuffer meshTransformBuffer, out VulkanBuffer propertiesBuffer);
                BufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId] = vertexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId] = indexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId] = meshTransformBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = propertiesBuffer;
                SpriteMeshMap[(int)meshId] = mesh;
            }
            finally
            {
                matrixHandle.Free();
                propertiesHandle.Free();
            }


            return (int)meshId;
        }

        public static int CreateLevelLayerMesh(Guid levelId, ListPtr<Vertex2D> vertexList, ListPtr<uint> indexList)
        {
            if (vertexList == null || vertexList.Count == 0 || indexList == null || indexList.Count == 0)
            {
                throw new ArgumentException("Vertex or index list is invalid.");
            }

            uint meshId = ++NextLevelLayerMeshId;
            mat4 meshMatrix = mat4.Identity;
            MeshPropertiesStruct meshProperties = new MeshPropertiesStruct { MeshTransform = meshMatrix };

            // Store vertex and index lists
            Vertex2DListMap[meshId] = vertexList;
            IndexListMap[meshId] = indexList;

            GCHandle matrixHandle = GCHandle.Alloc(meshMatrix, GCHandleType.Pinned);
            GCHandle propertiesHandle = GCHandle.Alloc(meshProperties, GCHandleType.Pinned);
            try
            {
                MeshLoader meshLoader = new MeshLoader
                {
                    MeshId = meshId,
                    ParentGameObjectID = 0,
                    MaterialId = Guid.Empty,
                    VertexLoader = new VertexLoaderStruct
                    {
                        VertexType = BufferTypeEnum.BufferType_Vector2D,
                        MeshVertexBufferId = (uint)(++BufferSystem.NextBufferId),
                        SizeofVertex = (size_t)sizeof(Vertex2D),
                        VertexCount = vertexList.Count,
                        VertexData = vertexList.Ptr
                    },
                    IndexLoader = new IndexLoaderStruct
                    {
                        MeshIndexBufferId = (uint)(++BufferSystem.NextBufferId),
                        SizeofIndex = (size_t)sizeof(uint),
                        IndexCount = indexList.Count,
                        IndexData = indexList.Ptr
                    },
                    TransformLoader = new TransformLoaderStruct
                    {
                        MeshTransformBufferId = (uint)(++BufferSystem.NextBufferId),
                        SizeofTransform = (size_t)sizeof(mat4),
                        TransformData = (void*)matrixHandle.AddrOfPinnedObject()
                    },
                    MeshPropertiesLoader = new MeshPropertiesLoaderStruct
                    {
                        PropertiesBufferId = (uint)(++BufferSystem.NextBufferId),
                        SizeofMeshProperties = (size_t)sizeof(MeshPropertiesStruct),
                        MeshPropertiesData = (void*)propertiesHandle.AddrOfPinnedObject()
                    }
                };

                ListPtr<Mesh> meshList = new ListPtr<Mesh>();
                meshList.Add(Mesh_CreateMesh(RenderSystem.renderer, meshLoader, out VulkanBuffer vertexBuffer, out VulkanBuffer indexBuffer, out VulkanBuffer meshTransformBuffer, out VulkanBuffer propertiesBuffer));
                LevelLayerMeshListMap[levelId] = meshList;

                BufferSystem.VulkanBufferMap[meshLoader.VertexLoader.MeshVertexBufferId] = vertexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.IndexLoader.MeshIndexBufferId] = indexBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.TransformLoader.MeshTransformBufferId] = meshTransformBuffer;
                BufferSystem.VulkanBufferMap[meshLoader.MeshPropertiesLoader.PropertiesBufferId] = propertiesBuffer;

                return (int)meshId;
            }
            finally
            {
                matrixHandle.Free();
                propertiesHandle.Free();
            }
        }

        public static void Update(float deltaTime)
        {
            foreach (var meshPair in SpriteMeshMap)
            {
                VulkanBuffer propertiesBuffer = BufferSystem.VulkanBufferMap[(uint)meshPair.Value.PropertiesBufferId];
                uint shaderMaterialBufferIndex = (meshPair.Value.MaterialId != new Guid()) ? MaterialSystem.MaterialMap[meshPair.Value.MaterialId].ShaderMaterialBufferIndex : 0;
                Mesh_UpdateMesh(RenderSystem.renderer, meshPair.Value, propertiesBuffer, shaderMaterialBufferIndex, deltaTime);
            }
        }

        public static void DestroyMesh(uint meshId)
        {
            if (!MeshMap.TryGetValue(meshId, out Mesh mesh))
                return;

            // Use int keys directly, as Mesh stores buffer IDs as int
            Mesh_DestroyMesh(RenderSystem.renderer, mesh,
                BufferSystem.VulkanBufferMap[(uint)mesh.MeshVertexBufferId],
                BufferSystem.VulkanBufferMap[(uint)mesh.MeshIndexBufferId],
                BufferSystem.VulkanBufferMap[(uint)mesh.MeshTransformBufferId],
                BufferSystem.VulkanBufferMap[(uint)mesh.PropertiesBufferId]);

            if (Vertex2DListMap.TryGetValue(meshId, out var vertexList))
            {
                vertexList.Dispose();
                Vertex2DListMap.Remove(meshId);
            }
            if (IndexListMap.TryGetValue(meshId, out var indexList))
            {
                indexList.Dispose();
                IndexListMap.Remove(meshId);
            }

            MeshMap.Remove(meshId);
        }

        public static Mesh FindMesh(uint meshId)
        {
            return MeshMap.Where(x => x.Key == meshId).First().Value;
        }

        public static Mesh FindSpriteMesh(uint meshId)
        {
            return SpriteMeshMap.Where(x => x.Key == meshId).First().Value;
        }

        public static ListPtr<Mesh> FindLevelLayerMeshList(Guid levelGuid)
        {
            return LevelLayerMeshListMap.Where(x => x.Key == levelGuid).First().Value;
        }

        public static ListPtr<Vertex2D> FindVertex2DList(uint meshId)
        {
            return Vertex2DListMap.Where(x => x.Key == meshId).First().Value;
        }

        public static ListPtr<uint> FindIndexList(uint meshId)
        {
            return IndexListMap.Where(x => x.Key == meshId).First().Value;
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)]  public static extern Mesh Mesh_CreateMesh(GraphicsRenderer renderer, MeshLoader meshLoader, out VulkanBuffer outVertexBuffer, out VulkanBuffer outIndexBuffer, out VulkanBuffer outTransformBuffer, out VulkanBuffer outPropertiesBuffer);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Mesh_UpdateMesh(GraphicsRenderer renderer, Mesh mesh, VulkanBuffer meshPropertiesBuffer, uint shaderMaterialBufferIndex, float deltaTime);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void Mesh_DestroyMesh(GraphicsRenderer renderer, Mesh mesh, VulkanBuffer vertexBuffer, VulkanBuffer indexBuffer, VulkanBuffer transformBuffer, VulkanBuffer propertiesBuffer);
    }
}
