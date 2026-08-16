using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace GameScriptLibraryDLL.GameObjects
{
    public unsafe static class CSharpScriptSystem
    {
        public static bool Initialize()
        {
            return DLLSystem.CallDLLFunc(() => CSharpScriptSystem_Initialize());
        }

        public static void RegisterBehavior<T>() where T : GameObject, IGameObjectType, new()
        {
            DLLSystem.CallDLLFunc(() =>
                GameObjectSystem_RegisterBehavior(
                    T.ObjectType,
                    GameObjectScript<T>.CreatePtr,
                    GameObjectScript<T>.StartupPtr,
                    GameObjectScript<T>.UpdatePtr,
                    GameObjectScript<T>.DestroyPtr));
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate IntPtr CreateObjectFn();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void StartupFn(IntPtr instance, uint gameObjectId, uint parentId);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void UpdateFn(IntPtr instance, float deltaTime);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void DestroyFn(IntPtr instance);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern bool CSharpScriptSystem_Initialize();
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void GameObjectSystem_RegisterBehavior(GameObjectTypeEnum gameObjectType, IntPtr create, IntPtr startup, IntPtr update, IntPtr destroy);
    }
}
