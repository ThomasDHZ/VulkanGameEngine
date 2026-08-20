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
        static readonly OnCollisionEnterFn s_onCollisionEnter = OnCollisionEnter;
        static readonly OnCollisionStayFn s_onCollisionStay = OnCollisionStay;
        static readonly OnCollisionExitFn s_onCollisionExit = OnCollisionExit;
        static readonly DestroyFn s_destroy = Destroy;

        static readonly Delegate[] s_roots = { s_create, s_startup, s_update, s_onCollisionEnter, s_onCollisionStay, s_onCollisionExit, s_destroy };

        static IntPtr Create() => GCHandle.ToIntPtr(GCHandle.Alloc(new T()));
        static void Startup(IntPtr instancePtr, uint id, uint parent) => FromPtr(instancePtr).StartUp(instancePtr, id, parent);
        static void Update(IntPtr instancePtr, float dt) => FromPtr(instancePtr).Update(instancePtr, dt);
        static void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => FromPtr(instancePtr).OnCollisionEnter(instancePtr, gameObjectId, collidingGameObjectId);
        static void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => FromPtr(instancePtr).OnCollisionStay(instancePtr, gameObjectId, collidingGameObjectId);
        static void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId) => FromPtr(instancePtr).OnCollisionExit(instancePtr, gameObjectId, collidingGameObjectId);
        static void Destroy(IntPtr instance)
        {
            FromPtr(instance).Destroy(instance);
            GCHandle.FromIntPtr(instance).Free();
        }

        static T FromPtr(IntPtr p) => (T)GCHandle.FromIntPtr(p).Target!;


        public static IntPtr CreatePtr => Marshal.GetFunctionPointerForDelegate(s_create);
        public static IntPtr StartupPtr => Marshal.GetFunctionPointerForDelegate(s_startup);
        public static IntPtr UpdatePtr => Marshal.GetFunctionPointerForDelegate(s_update);
        public static IntPtr OnCollisionEnterPtr => Marshal.GetFunctionPointerForDelegate(s_onCollisionEnter);
        public static IntPtr OnCollisionStayPtr => Marshal.GetFunctionPointerForDelegate(s_onCollisionStay);
        public static IntPtr OnCollisionExitPtr => Marshal.GetFunctionPointerForDelegate(s_onCollisionExit);
        public static IntPtr DestroyPtr => Marshal.GetFunctionPointerForDelegate(s_destroy);
    }
}
