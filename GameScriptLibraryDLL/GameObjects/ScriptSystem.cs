using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static GameScriptLibraryDLL.GameObjects.CSharpScriptSystem;
using static GameScriptLibraryDLL.GameObjects.GameObject;

namespace GameScriptLibraryDLL.GameObjects
{
    public static class GameObjectScript<T> where T : GameObject, new()
    {
        static readonly CreateObjectFn s_create = CreateNative;
        static readonly StartupFn s_startup = StartupNative;
        static readonly UpdateFn s_update = UpdateNative;
        static readonly DestroyFn s_destroy = DestroyNative;
        static readonly Delegate[] s_roots = { s_create, s_startup, s_update, s_destroy };

        static IntPtr CreateNative() => GCHandle.ToIntPtr(GCHandle.Alloc(new T()));

        static void StartupNative(IntPtr instance, uint id, uint parent)
            => FromPtr(instance).StartUp(instance, id, parent);

        static void UpdateNative(IntPtr instance, float dt)
            => FromPtr(instance).Update(instance, dt);

        static void DestroyNative(IntPtr instance)
        {
            FromPtr(instance).Destroy(instance);
            GCHandle.FromIntPtr(instance).Free();
        }

        static T FromPtr(IntPtr p) => (T)GCHandle.FromIntPtr(p).Target!;

        public static IntPtr CreatePtr => Marshal.GetFunctionPointerForDelegate(s_create);
        public static IntPtr StartupPtr => Marshal.GetFunctionPointerForDelegate(s_startup);
        public static IntPtr UpdatePtr => Marshal.GetFunctionPointerForDelegate(s_update);
        public static IntPtr DestroyPtr => Marshal.GetFunctionPointerForDelegate(s_destroy);
    }
}
