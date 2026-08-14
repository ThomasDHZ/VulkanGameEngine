using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static GameScriptLibraryDLL.GameObjects.GameObject;

namespace GameScriptLibraryDLL.GameObjects
{
    public unsafe class GameObjectScript<T> where T : GameObject, new()
    {
        protected static T FromPtr(IntPtr p) => (T)GCHandle.FromIntPtr(p).Target!;
        protected static IntPtr ToPtr(object obj) => GCHandle.ToIntPtr(GCHandle.Alloc(obj));
        protected static void FreePtr(IntPtr p) => GCHandle.FromIntPtr(p).Free();
        public static IntPtr CreateNativePtr() => ToPtr(new T());
        public static void StartupNativePtr(IntPtr instance, uint id, uint parent) => FromPtr(instance).StartUp(instance, id, parent);
        public static void UpdateNativePtr(IntPtr instance, float dt) => FromPtr(instance).Update(instance, dt);
        public static void DestroyNativePtr(IntPtr instance)
        {
            FromPtr(instance).Destroy(instance);
            FreePtr(instance);
        }

        // Root delegates for this T (one static instance per closed generic)
        static readonly CreateObjectFn s_create = CreateNativePtr;
        static readonly StartupFn s_startup = StartupNativePtr;
        static readonly UpdateFn s_update = UpdateNativePtr;
        static readonly DestroyFn s_destroy = DestroyNativePtr;

        public static nint CreatePtr => Marshal.GetFunctionPointerForDelegate(s_create);
        public static nint StartupPtr => Marshal.GetFunctionPointerForDelegate(s_startup);
        public static nint UpdatePtr => Marshal.GetFunctionPointerForDelegate(s_update);
        public static nint DestroyPtr => Marshal.GetFunctionPointerForDelegate(s_destroy);

        public static void Register()
        {
            CSharpScriptSystem.RegisterBehavior<T>();
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate IntPtr CreateObjectFn();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void StartupFn(IntPtr instance, uint gameObjectId, uint parentId);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void UpdateFn(IntPtr instance, float deltaTime);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void DestroyFn(IntPtr instance);
    }
}
