using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public static unsafe class LevelEditorSystem
    {
        public static void SetSelectedGameObject(UInt32 gameObjectId)
        {
            DLLSystem.CallDLLFunc(() => LevelEditorSystem_SetSelectedGameObject(gameObjectId));
        }

        public static uint SampleRenderPassPixel(Guid textureGuid, ivec2 mousePosition)
        {
            return DLLSystem.CallDLLFunc(() => LevelEditorSystem_SampleRenderPassPixel(textureGuid, mousePosition));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelEditorSystem_SetSelectedGameObject(uint gameObjectId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint LevelEditorSystem_SampleRenderPassPixel(Guid textureGuid, ivec2 mousePosition);
    }
}
