﻿using Coral.Managed.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineGameObjectScripts.Input;
using VulkanGameEngineGameObjectScripts.Interface;
using VulkanGameEngineGameObjectScripts;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using GlmSharp;
using Newtonsoft.Json;

namespace VulkanGameEngineLevelEditor.Components
{
    public unsafe class InputComponent : GameObjectComponent
    {
        Transform2DComponent transform;

        public InputComponent() : base()
        {
            Name = "InputComponent";
            ComponentType = ComponentTypeEnum.kInputComponent;
        }

        public InputComponent(uint gameObjectId) : base(gameObjectId, ComponentTypeEnum.kInputComponent)
        {
            Name = "InputComponent";
            transform = ParentGameObject.GameObjectComponentList.Where(x => x.ComponentType == ComponentTypeEnum.kGameObjectTransform2DComponent).First() as Transform2DComponent;
        }

        public InputComponent(uint gameObjectId, string name) : base(gameObjectId, name, ComponentTypeEnum.kInputComponent)
        {
            transform = ParentGameObject.GameObjectComponentList.Where(x => x.ComponentType == ComponentTypeEnum.kGameObjectTransform2DComponent).First() as Transform2DComponent;
        }

        public override void Destroy()
        {

        }

        public override void Draw(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, ListPtr<VkDescriptorSet> descriptorSetList, SceneDataBuffer sceneProperties)
        {

        }

        public override int GetMemorySize()
        {
            return sizeof(InputComponent);
        }

        //public override void Input(KeyBoardKeys key, float deltaTime)
        //{
        //    if (transform != null)
        //    {
        //        if (key == KeyBoardKeys.W)
        //        {
        //            transform.GameObjectPosition.y += 0.01f;
        //        }
        //        if (key == KeyBoardKeys.A)
        //        {
        //            transform.GameObjectPosition.x -= 0.01f;
        //        }
        //        if (key == KeyBoardKeys.S)
        //        {
        //            transform.GameObjectPosition.y -= 0.01f;
        //        }
        //        if (key == KeyBoardKeys.D)
        //        {
        //            transform.GameObjectPosition.x += 0.01f;
        //        }
        //    }
        //    else
        //    {
        //        Console.WriteLine($"Transform not found: GameObjectId: 0x{CPPcomponentPtr.ToString("x")} TransformID: 0x{CPPcomponentPtr.ToString("x")}");
        //    }
        //}

        public void Input(int key, float deltaTime)
        {
            if (transform != null)
            {
                if ((KeyBoardKeys)key == KeyBoardKeys.W)
                {
                  //  transform.GameObjectPosition.y += 0.01f;
                }
                if ((KeyBoardKeys)key == KeyBoardKeys.A)
                {
                  //  transform.GameObjectPosition.x -= 0.01f;
                }
                if ((KeyBoardKeys)key == KeyBoardKeys.S)
                {
                  //  transform.GameObjectPosition.y -= 0.01f;
                }
                if ((KeyBoardKeys)key == KeyBoardKeys.D)
                {
                  //  transform.GameObjectPosition.x += 0.01f;
                }
            }
            else
            {
              //  Console.WriteLine($"Transform not found: GameObjectId: 0x{CPPcomponentPtr.ToString("x")} TransformID: 0x{CPPcomponentPtr.ToString("x")}");
            }
        }

        public override void Update(VkCommandBuffer commandBuffer, float deltaTime)
        {

        }

    }
}
