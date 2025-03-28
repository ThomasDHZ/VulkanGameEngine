﻿using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using VulkanGameEngineGameObjectScripts;
using VulkanGameEngineGameObjectScripts.Input;
using VulkanGameEngineLevelEditor.Components;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    //RenderMesh2DComponent is basically just a skeletion container for linking from C++ to C#. 
    //Mesh and Draw calls are too diffrent to cleanly run them on C#.
    //public unsafe class MeshRenderer2DComponent //: RenderMesh2DComponent
    //{
    //    public Mesh2D mesh { get; protected set; } = new Mesh2D();
    //    public MeshRenderer2DComponent()
    //    {

    //    }

    //    public MeshRenderer2DComponent(IntPtr parentGameObject, String name, uint meshBufferIndex)
    //    {
    //        List<Vertex2D> spriteVertexList = new List<Vertex2D>
    //        {
    //            new Vertex2D(new vec2(0.0f, 0.5f), new vec2(0.0f, 0.0f), new vec4(1.0f, 0.0f, 0.0f, 1.0f)),
    //            new Vertex2D(new vec2(0.5f, 0.5f), new vec2(1.0f, 0.0f), new vec4(0.0f, 1.0f, 0.0f, 1.0f)),
    //            new Vertex2D(new vec2(0.5f, 0.0f), new vec2(1.0f, 1.0f), new vec4(0.0f, 0.0f, 1.0f, 1.0f)),
    //            new Vertex2D(new vec2(0.0f, 0.0f), new vec2(0.0f, 1.0f), new vec4(1.0f, 1.0f, 0.0f, 1.0f))
    //        };

    //        List<uint> spriteIndexList = new List<uint> { 0, 1, 3, 1, 2, 3 };

    //        mesh = new Mesh2D(spriteVertexList, spriteIndexList, null);
    //    }

    //    public override void Input(KeyBoardKeys key, float deltaTime)
    //    {

    //    }

    //    public override void Update(VkCommandBuffer commandBuffer, float deltaTime)
    //    {
    //        mesh.Update(commandBuffer, deltaTime);
    //    }

    //    public override void Draw(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, ListPtr<VkDescriptorSet> descriptorSetList, SceneDataBuffer sceneProperties)
    //    {
    //        mesh.Draw(commandBuffer, pipeline, pipelineLayout, descriptorSetList, sceneProperties);
    //    }

    //    public override void Destroy()
    //    {
    //        mesh.Destroy();
    //    }

    //    public override int GetMemorySize()
    //    {
    //        return (int)sizeof(MeshRenderer2DComponent);
    //    }
    //}
}