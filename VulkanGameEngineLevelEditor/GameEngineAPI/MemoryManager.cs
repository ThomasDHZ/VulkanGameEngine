﻿using Newtonsoft.Json.Linq;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Buffers;
using System.Collections.Generic;
using VulkanGameEngineLevelEditor.RenderPassEditor;
using VulkanGameEngineLevelEditor.Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public unsafe class MemoryManager
    {
        public static Vk vk = Vk.GetApi();
        public static MemoryPool<GameObject> GameObjectMemoryPool = new MemoryPool<GameObject>();
        public static MemoryPool<MeshRenderer2DComponent> RenderMesh2DComponentMemoryPool = new MemoryPool<MeshRenderer2DComponent>();
        public static MemoryPool<MeshRenderer3DComponent> RenderMesh3DComponentMemoryPool = new MemoryPool<MeshRenderer3DComponent>();
        public static MemoryPool<Texture> TextureMemoryPool = new MemoryPool<Texture>();

        public static void StartUp(uint estObjectCount)
        {
            GameObjectMemoryPool.CreateMemoryPool(estObjectCount);
            RenderMesh2DComponentMemoryPool.CreateMemoryPool(estObjectCount);
            RenderMesh3DComponentMemoryPool.CreateMemoryPool(estObjectCount);
            TextureMemoryPool.CreateMemoryPool(estObjectCount);
        }

        public static GameObject AllocateGameObject()
        {
            return GameObjectMemoryPool.AllocateMemoryLocation();
        }

        public static MeshRenderer2DComponent AllocateGameRenderMesh2DComponent()
        {
            return RenderMesh2DComponentMemoryPool.AllocateMemoryLocation();
        }

        public static MeshRenderer3DComponent AllocateGameRenderMesh3DComponent()
        {
            return RenderMesh3DComponentMemoryPool.AllocateMemoryLocation();
        }

        public static Texture AllocateTexture()
        {
            return TextureMemoryPool.AllocateMemoryLocation();
        }

        public static List<DescriptorBufferInfo> GetGameObjectPropertiesBuffer()
        {
            var renderMesh3DMemory = RenderMesh3DComponentMemoryPool.ViewMemoryPool();

            List<DescriptorBufferInfo> MeshPropertiesBuffer = new List<DescriptorBufferInfo>();
            if (renderMesh3DMemory.Count == 0)
            {
                DescriptorBufferInfo nullBuffer = new DescriptorBufferInfo();
                nullBuffer.Buffer = new Silk.NET.Vulkan.Buffer();
                nullBuffer.Offset = 0;
                nullBuffer.Range = Vk.WholeSize;
                MeshPropertiesBuffer.Add(nullBuffer);
            }
            else
            {
                foreach (var mesh in renderMesh3DMemory)
                {
                    if (mesh != null)
                    {
                        DescriptorBufferInfo MeshProperitesBufferInfo = new DescriptorBufferInfo();
                        MeshProperitesBufferInfo.Buffer = mesh.GetMeshPropertiesBuffer().Buffer;
                        MeshProperitesBufferInfo.Offset = 0;
                        MeshProperitesBufferInfo.Range = mesh.GetMeshPropertiesBuffer().BufferSize;
                        MeshPropertiesBuffer.Add(MeshProperitesBufferInfo);
                    }
                }
            }

            return MeshPropertiesBuffer;
        }

        public static List<DescriptorImageInfo> GetTexturePropertiesBuffer()
        {
            List<DescriptorImageInfo> TexturePropertiesBuffer = new List<DescriptorImageInfo>();
            var textureMemoryList = TextureMemoryPool.ViewMemoryPool();
            if (textureMemoryList.Count == 0)
            {
                SamplerCreateInfo NullSamplerInfo = new SamplerCreateInfo();
                NullSamplerInfo.SType = StructureType.SamplerCreateInfo;
                NullSamplerInfo.MagFilter = Filter.Nearest;
                NullSamplerInfo.MinFilter = Filter.Nearest;
                NullSamplerInfo.AddressModeU = SamplerAddressMode.Repeat;
                NullSamplerInfo.AddressModeV = SamplerAddressMode.Repeat;
                NullSamplerInfo.AddressModeW = SamplerAddressMode.Repeat;
                NullSamplerInfo.AnisotropyEnable = Vk.True;
                NullSamplerInfo.MaxAnisotropy = 16.0f;
                NullSamplerInfo.BorderColor = BorderColor.FloatOpaqueBlack;
                NullSamplerInfo.UnnormalizedCoordinates = Vk.False;
                NullSamplerInfo.CompareEnable = Vk.False;
                NullSamplerInfo.CompareOp = CompareOp.Always;
                NullSamplerInfo.MipmapMode = SamplerMipmapMode.Linear;
                NullSamplerInfo.MinLod = 0;
                NullSamplerInfo.MaxLod = 0;
                NullSamplerInfo.MipLodBias = 0;

                Sampler nullSampler = new Sampler();
                var result = vk.CreateSampler(VulkanRenderer.device, &NullSamplerInfo, null, &nullSampler);


                DescriptorImageInfo nullBuffer = new DescriptorImageInfo();
                nullBuffer.ImageLayout = ImageLayout.ShaderReadOnlyOptimal;
                nullBuffer.ImageView = new ImageView();
                nullBuffer.Sampler = nullSampler;
                TexturePropertiesBuffer.Add(nullBuffer);
            }
            else
            {
                foreach (var texture in textureMemoryList)
                {
                    if (texture != null)
                    {
                        DescriptorImageInfo textureDescriptor = new DescriptorImageInfo();
                        textureDescriptor.ImageLayout = ImageLayout.ShaderReadOnlyOptimal;
                        textureDescriptor.ImageView = texture.View;
                        textureDescriptor.Sampler = texture.Sampler;
                        TexturePropertiesBuffer.Add(textureDescriptor);
                    }
                }
            }

            return TexturePropertiesBuffer;
        }


        public static void ViewMemoryMap()
        {
            var gameObjectMemoryList = GameObjectMemoryPool.ViewMemoryPool();
            var renderMesh2DMemoryList = RenderMesh2DComponentMemoryPool.ViewMemoryPool();
            var renderMesh3DMemoryList = RenderMesh3DComponentMemoryPool.ViewMemoryPool();
            var textureMemoryList = TextureMemoryPool.ViewMemoryPool();

            Console.WriteLine($"Memory Map of Game Objects:");
            Console.WriteLine("{0,20} {1,15}", "Index", "Value");
            for (int x = 0; x < GameObjectMemoryPool.ObjectCount; x++)
            {
                var gameObjectMemoryListRef = gameObjectMemoryList[x];
                GameObject* gameObjectPtr = &gameObjectMemoryListRef;

                IntPtr address = (IntPtr)gameObjectPtr + (sizeof(GameObject) * x);

                string value = gameObjectMemoryList[x]?.ToString() ?? "null";
                Console.WriteLine($"{x,10} : {address.ToString("X12")} : {value}");
            }
            Console.WriteLine();

            Console.WriteLine($"Memory Map of RenderMesh2DComponent:");
            Console.WriteLine("{0,20} {1,15}", "Index", "Value");
            for (int x = 0; x < RenderMesh2DComponentMemoryPool.ObjectCount; x++)
            {
                var render2DMemoryListRef = renderMesh2DMemoryList[x];
                MeshRenderer2DComponent* render2DPtr = &render2DMemoryListRef;

                IntPtr address = (IntPtr)render2DPtr + (sizeof(MeshRenderer2DComponent) * x);

                string value = renderMesh2DMemoryList[x]?.ToString() ?? "null";
                Console.WriteLine($"{x,10} : {address.ToString("X12")} : {value}");
            }

            Console.WriteLine();

            Console.WriteLine($"Memory Map of RenderMesh2DComponent:");
            Console.WriteLine("{0,20} {1,15}", "Index", "Value");
            for (int x = 0; x < RenderMesh2DComponentMemoryPool.ObjectCount; x++)
            {
                var render2DMemoryListRef = renderMesh2DMemoryList[x];
                MeshRenderer2DComponent* render2DPtr = &render2DMemoryListRef;

                IntPtr address = (IntPtr)render2DPtr + (sizeof(MeshRenderer2DComponent) * x);

                string value = renderMesh2DMemoryList[x]?.ToString() ?? "null";
                Console.WriteLine($"{x,10} : {address.ToString("X12")} : {value}");
            }

            Console.WriteLine($"Memory Map of RenderMesh3DComponent:");
            Console.WriteLine("{0,20} {1,15}", "Index", "Value");
            for (int x = 0; x < RenderMesh3DComponentMemoryPool.ObjectCount; x++)
            {
                var render3DMemoryListRef = renderMesh3DMemoryList[x];
                MeshRenderer3DComponent* render3DPtr = &render3DMemoryListRef;

                IntPtr address = (IntPtr)render3DPtr + (sizeof(MeshRenderer3DComponent) * x);

                string value = renderMesh3DMemoryList[x]?.ToString() ?? "null";
                Console.WriteLine($"{x,10} : {address.ToString("X12")} : {value}");
            }

            Console.WriteLine();
        }
    }
}