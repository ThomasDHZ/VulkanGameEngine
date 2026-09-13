using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Models;

namespace VulkanEngineCS
{
    public unsafe class IblRenderSystem
    {
        public static void StartUp(string texturePath)
        {
            DLLSystem.CallDLLFunc(() => IblRenderSystem_StartUp(texturePath));
        }

        public static List<RenderPassNode> CreateDrawCommands(VkCommandBuffer commandBuffer, float deltaTime)
        {
            List<RenderPassNode> renderPassNodeList = new List<RenderPassNode>();
            RenderPassNodeDLL* nodes = IblRenderSystem_CreateDrawCommands(ref commandBuffer, deltaTime, out size_t count);
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

        void SetEnvironmentMap(string texturePath)
        {
            DLLSystem.CallDLLFunc(() => IblRenderSystem_SetEnvironmentMap(texturePath));
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void IblRenderSystem_StartUp([MarshalAs(UnmanagedType.LPStr)] string texturePath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern RenderPassNodeDLL* IblRenderSystem_CreateDrawCommands(ref VkCommandBuffer commandBuffer, float deltaTime, out size_t renderPassNodeCount);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void IblRenderSystem_SetEnvironmentMap([MarshalAs(UnmanagedType.LPStr)] string texturePath);
    }
}
