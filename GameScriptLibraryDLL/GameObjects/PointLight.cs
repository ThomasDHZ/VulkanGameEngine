using GameScriptLibraryDLL.Components;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.GameObjects
{
    public unsafe class PointLight : GameObject, IGameObjectType
    {
        public static GameObjectTypeEnum ObjectType => GameObjectTypeEnum.kGameObjectPointLight;

        public override IntPtr Create()
        {
            var instance = new PointLight();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        public override void StartUp(IntPtr instancePtr, uint gameObjectId, uint parentGameObjectId)
        {
        }

        public override void OnCollisionEnter(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void OnCollisionStay(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void OnCollisionExit(IntPtr instancePtr, uint gameObjectId, uint collidingGameObjectId)
        {
        }

        public override void Update(IntPtr instancePtr, float deltaTime)
        {

        }

        public override void Destroy(IntPtr instance)
        {
            if (instance != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instance);
                handle.Free();
            }
        }
    }
}
