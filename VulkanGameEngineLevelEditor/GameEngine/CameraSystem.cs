using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public struct Camera
    {
        public float Width { get; set; }
        public float Height { get; set; }
        public float AspectRatio { get; set; }
        public float Yaw { get; set; }
        public float Pitch { get; set; }
        public float Zoom { get; set; }
        public float ZNear { get; set; } = 0.1f;
        public float ZFar { get; set; } = 10000.0f;

        public float MovementSpeed { get; set; }
        public float MouseSensitivity { get; set; }

        public vec3 Front { get; set; }
        public vec3 Up { get; set; }
        public vec3 Right { get; set; }
        public vec3 WorldUp { get; set; }

        public vec3 Position { get; set; }
        public vec2 ViewScreenSize { get; set; }
        public mat4 ProjectionMatrix { get; set; }
        public mat4 ViewMatrix { get; set; }
        public Camera()
        {
        }
    };

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

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern IntPtr CameraSystem_UpdateActiveCamera();
    }
}
