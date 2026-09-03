using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;

namespace VulkanEngineCS
{
    public static unsafe class CameraSystem
    {
        public static ref Camera UpdateActiveCamera()
        {
            IntPtr ptr = CameraSystem_UpdateActiveCamera();
            if (ptr == IntPtr.Zero)
            {
                return ref Unsafe.NullRef<Camera>();
            }
            return ref Unsafe.AsRef<Camera>(ptr.ToPointer());
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.StdCall)] private static extern IntPtr CameraSystem_UpdateActiveCamera();
    }
}
