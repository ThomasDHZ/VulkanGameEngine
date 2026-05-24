using System.Runtime.InteropServices;

namespace GameScriptLibraryDLL
{
    public class Player
    {
        // Define delegates
        public delegate IntPtr CreateDelegate();
        public delegate void StartUpDelegate(IntPtr instance);
        public delegate void UpdateDelegate(IntPtr instance, float deltaTime);
        public delegate void DestroyDelegate(IntPtr instance);

        [UnmanagedCallersOnly]
        public static IntPtr Create()
        {
            Console.WriteLine("[C#] Player_Create called from native!");
            var instance = new Player();
            GCHandle handle = GCHandle.Alloc(instance, GCHandleType.Normal);
            return GCHandle.ToIntPtr(handle);
        }

        [UnmanagedCallersOnly]
        public static void StartUp(IntPtr instance)
        {
            Console.WriteLine("[C#] Player.StartUp called");
        }

        [UnmanagedCallersOnly]
        public static void Update(IntPtr instance, float deltaTime)
        {
             Console.WriteLine($"[C#] Player.Update({deltaTime})");
        }

        [UnmanagedCallersOnly]
        public static void Destroy(IntPtr instance)
        {
            Console.WriteLine("[C#] Player.Destroy called");

            if (instance != IntPtr.Zero)
            {
                GCHandle handle = GCHandle.FromIntPtr(instance);
                handle.Free();                    // ← Must free!
            }
        }
    }
}