using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Models;
using VulkanEngineCoreCS.Vulkan;

namespace VulkanEngineCS
{
    public unsafe class LevelSystem
    {
        public static void LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }

        public static void RenderFrameBuffer(VkCommandBuffer commandBuffer, Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderFrameBuffer(ref commandBuffer, renderPassId));
        }

        public static List<RenderPassNode> CreateDrawCommands(VkCommandBuffer commandBuffer, float deltaTime)
        {
            List<RenderPassNode> renderPassNodeList = new List<RenderPassNode>();
            RenderPassNodeDLL* nodes = LevelSystem_CreateDrawCommands(ref commandBuffer, deltaTime, out size_t count);
            if (nodes == null || count == 0) return new List<RenderPassNode>();

            for (size_t x = 0; x < count; x++)
            {
                List<List<VulkanDrawMessage>> subPassDrawList = new List<List<VulkanDrawMessage>>();
                ref RenderPassNodeDLL node = ref nodes[x];
                for (size_t y = 0; y < node.SubPassDrawMessage_RenderPassCount; y++)
                {
                    size_t drawCount = node.SubPassDrawMessage_SubPassCounts[y];
                    VulkanDrawMessageDLL* draws = node.SubPassDrawMessage[y];

                    List<VulkanDrawMessage> drawMeshMessage = new List<VulkanDrawMessage>();
                    for (size_t z = 0; z < drawCount; z++)
                    {
                        drawMeshMessage.Add(VulkanDrawMessage.FromDLL(draws[z]));
                    }
                    subPassDrawList.Add(drawMeshMessage);
                }
                renderPassNodeList.Add(new RenderPassNode
                {
                    RenderPassGuid = node.RenderPassGuid,
                    MipCount = node.MipCount,
                    PostRenderPassCmd = node.PostRenderPassCmd,
                    PreRenderPassCmd = node.PreRenderPassCmd,
                    SubPassDrawMessage = subPassDrawList,
                });
            }
            return renderPassNodeList;
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_RenderFrameBuffer(ref VkCommandBuffer commandBuffer, Guid renderPassId);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern RenderPassNodeDLL* LevelSystem_CreateDrawCommands(ref VkCommandBuffer commandBuffer, float deltaTime, out size_t renderPassNodeDllCount);
    }
}
