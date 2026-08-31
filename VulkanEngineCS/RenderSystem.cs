using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Models;
using VulkanEngineCoreCS.Vulkan;

namespace VulkanEngineCS
{
    public unsafe class RenderSystem
    {
        public Guid HdrRenderPassId { get; } = new Guid();
        public static Guid LoadRenderPass(string jsonPath)
        {
            return DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(jsonPath));
        }

        public static void Update(void* windowHandle, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Update(windowHandle, deltaTime));
        }
        public static void PresentToSwapChain(VkCommandBuffer commandBuffer, Guid renderPassTextureGuid)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_PresentToSwapChain(ref commandBuffer, renderPassTextureGuid));
        }

        public static unsafe void Draw(VkCommandBuffer commandBuffer, List<RenderPassNode> renderPassNodes)
        {
            if (renderPassNodes == null || renderPassNodes.Count == 0) return;

            size_t count = (size_t)renderPassNodes.Count;
            RenderPassNodeDLL* nodeArray = (RenderPassNodeDLL*)Marshal.AllocHGlobal(sizeof(RenderPassNodeDLL) * renderPassNodes.Count);
            try
            {
                for (int x = 0; x < renderPassNodes.Count; x++)
                {
                    var srcNode = renderPassNodes[x];
                    ref RenderPassNodeDLL dstNode = ref nodeArray[x];

                    size_t* subPassCounts = null;
                    VulkanDrawMessageDLL** subPassPtrs = null;
                    size_t subPassCount = (size_t)srcNode.SubPassDrawMessage.Count;
                    if (subPassCount > 0) subPassCounts = (size_t*)Marshal.AllocHGlobal(sizeof(size_t) * (int)subPassCount);
                    if (subPassCount > 0) subPassPtrs = (VulkanDrawMessageDLL**)Marshal.AllocHGlobal(sizeof(VulkanDrawMessageDLL*) * (int)subPassCount);
                    for (int y = 0; y < (int)subPassCount; y++)
                    {
                        var srcDrawList = srcNode.SubPassDrawMessage[y];
                        size_t drawCount = (size_t)srcDrawList.Count;
                        subPassCounts[y] = drawCount;

                        if (drawCount == 0)
                        {
                            subPassPtrs[y] = null;
                            continue;
                        }

                        VulkanDrawMessageDLL* drawArray = (VulkanDrawMessageDLL*)Marshal.AllocHGlobal(sizeof(VulkanDrawMessageDLL) * (int)drawCount);
                        for (int z = 0; z < (int)drawCount; z++)
                        {
                            drawArray[z] = VulkanDrawMessageDLL.ToDLL(srcDrawList[z]);
                        }
                        subPassPtrs[y] = drawArray;
                    }
                    dstNode = new RenderPassNodeDLL
                    {
                        RenderPassGuid = srcNode.RenderPassGuid,
                        SubPassDrawMessage = subPassPtrs,
                        SubPassDrawMessage_RenderPassCount = subPassCount,
                        SubPassDrawMessage_SubPassCounts = subPassCounts,
                        PreRenderPassCmd = srcNode.PreRenderPassCmd,
                        PostRenderPassCmd = srcNode.PostRenderPassCmd,
                        MipCount = srcNode.MipCount
                    };
                }
                RenderSystem_Draw(ref commandBuffer, nodeArray, count);
            }
            finally
            {
                for (size_t x = 0; x < count; x++)
                {
                    ref RenderPassNodeDLL node = ref nodeArray[x];
                    size_t subPassCount = node.SubPassDrawMessage_RenderPassCount;
                    if (node.SubPassDrawMessage != null && subPassCount > 0)
                    {
                        for (size_t y = 0; y < subPassCount; y++)
                        {
                            if (node.SubPassDrawMessage[y] != null)
                            {
                                Marshal.FreeHGlobal((IntPtr)node.SubPassDrawMessage[y]);
                            }
                        }
                        Marshal.FreeHGlobal((IntPtr)node.SubPassDrawMessage);
                    }
                    if (node.SubPassDrawMessage_SubPassCounts != null) Marshal.FreeHGlobal((IntPtr)node.SubPassDrawMessage_SubPassCounts);
                }
                Marshal.FreeHGlobal((IntPtr)nodeArray);
            }
        }

        public static List<Guid> FindRenderPassAttachmentList(Guid renderPassGuid)
        {
            Guid* ptrList = RenderSystem_FindRenderPassAttachmentList(renderPassGuid, out uint count);
            try
            {
                List<Guid> attachmentIdList = new List<Guid>();
                for (int x = 0; x < count; x++)
                {
                    ref Guid attachmentId = ref ptrList[x];
                    attachmentIdList.Add(attachmentId);
                }
                return attachmentIdList;
            }
            finally
            {
                MemorySystem.DeletePtr<Guid>(ptrList);
            }
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern Guid RenderSystem_LoadRenderPass([MarshalAs(UnmanagedType.LPStr)] string jsonPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void RenderSystem_Update(void* windowHandle, float deltaTime);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void RenderSystem_PresentToSwapChain(ref VkCommandBuffer commandBuffer, Guid renderPassTextureGuid);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void RenderSystem_Draw(ref VkCommandBuffer commandBuffer, RenderPassNodeDLL* renderPassNodeListPtr, size_t renderPassNodeCount);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern Guid* RenderSystem_FindRenderPassAttachmentList(Guid renderPassGuid, out uint returnTextureCount);
    }
}
