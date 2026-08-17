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

        public static void Draw(VkCommandBuffer commandBuffer, float deltaTime)
        {
            RenderPassNodeDLL* nodes = LevelSystem_Draw(ref commandBuffer, deltaTime, out size_t count);
            if (nodes == null || count == 0)  return;

            for (size_t x = 0; x < count; x++)
            {
                ref RenderPassNodeDLL node = ref nodes[x];

                IntPtr preCmd = node.PreRenderPassCmd;
                IntPtr postCmd = node.PostRenderPassCmd;

                for (size_t s = 0; s < node.SubPassDrawMessage_RenderPassCount; s++)
                {
                    size_t drawCount = node.SubPassDrawMessage_SubPassCounts[s];
                    VulkanDrawMessageDLL* draws = node.SubPassDrawMessage[s];

                    List<VulkanDrawMessage> drawMeshMessage = new List<VulkanDrawMessage>();
                    for (size_t d = 0; d < drawCount; d++)
                    {
                        drawMeshMessage.Add(VulkanDrawMessage.FromDLL(draws[d]));
                    }
                }
            }
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern RenderPassNodeDLL* LevelSystem_Draw(ref VkCommandBuffer commandBuffer, float deltaTime, out size_t renderPassNodeDllCount);
    }
}
