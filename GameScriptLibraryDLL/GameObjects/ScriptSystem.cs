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
        static readonly CreateObjectFn s_create = Create;
        static readonly StartupFn s_startup = Startup;
        static readonly UpdateFn s_update = Update;
        static readonly DestroyFn s_destroy = Destroy;
        static readonly Delegate[] s_roots = { s_create, s_startup, s_update, s_destroy };

        static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new T()));
        static void Startup(IntPtr instance, uint id, uint parent) => FromPtr(instance).StartUp(instance, id, parent);
        static void Update(IntPtr instance, float dt)
        {
            Console.WriteLine("GameObject Update called");
            FromPtr(instance).Update(instance, dt);
        }

        static void Destroy(IntPtr instance)
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
