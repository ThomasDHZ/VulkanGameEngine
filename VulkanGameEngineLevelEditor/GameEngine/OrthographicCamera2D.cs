using GlmSharp;
using System;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public unsafe class OrthographicCamera2D : Camera
    {
        public OrthographicCamera2D()
        {

        }

        public OrthographicCamera2D(float width, float height)
        {
            Position = new vec3(0.0f);
            ViewScreenSize = new vec2(width, height);
            ProjectionMatrix = mat4.Ortho(0.0f, Width, Height, 0.0f);
            ViewMatrix = mat4.Identity;
        }

        public OrthographicCamera2D(vec2 viewScreenSize)
        {
            Width = viewScreenSize.x;
            Height = viewScreenSize.y;
            AspectRatio = viewScreenSize.x / viewScreenSize.y;
            Zoom = 1.0f;

            Position = new vec3(0.0f);
            ViewScreenSize = viewScreenSize;
            ProjectionMatrix = mat4.Ortho(0.0f, Width, Height, 0.0f);
            ViewMatrix = mat4.Identity;
        }

        public OrthographicCamera2D(vec2 viewScreenSize, vec2 position)
        {
            Width = viewScreenSize.x;
            Height = viewScreenSize.y;
            AspectRatio = viewScreenSize.x / viewScreenSize.y;
            Zoom = 1.0f;

            Position = new vec3(position, 0.0f);
            ViewScreenSize = viewScreenSize;
            ProjectionMatrix = mat4.Ortho(0.0f, Width, Height, 0.0f);
            ViewMatrix = mat4.Identity;
        }

        public override void UpdateKeyboard(float deltaTime)
        {
            throw new NotImplementedException();
        }

        public override void UpdateMouse()
        {
            throw new NotImplementedException();
        }

        public override unsafe void Update(ShaderPushConstant sceneDataBuffer)
        {
            if (sceneDataBuffer.PushConstantBuffer == null)
            {
                sceneDataBuffer.PushConstantBuffer = (void*)Marshal.AllocHGlobal((int)sceneDataBuffer.PushConstantSize);
                byte* ptr = (byte*)sceneDataBuffer.PushConstantBuffer;
                for (int i = 0; i < (int)sceneDataBuffer.PushConstantSize; i++)
                {
                    ptr[i] = 0;
                }
            }

            mat4 transform = mat4.Translate(Position) * mat4.Rotate(0.0f, new vec3(0, 0, 1));
            ViewMatrix = transform.Inverse;
            ProjectionMatrix = mat4.Ortho(0.0f, Width, Height, 0.0f);

            ShaderVariable* shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "MeshBufferIndex");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                uint* ptr = (uint*)shaderVar->Value;
                *ptr = 0; // Match C++ default
            }

            shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "Projection");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* matrixPtr = (float*)shaderVar->Value;
                for (int x = 0; x < 16; x++)
                {
                    matrixPtr[x] = ProjectionMatrix[x];
                }
            }

            shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "View");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* matrixPtr = (float*)shaderVar->Value;
                for (int x = 0; x < 16; x++)
                {
                    matrixPtr[x] = ViewMatrix[x];
                }
            }

            shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "CameraPosition");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* ptr = (float*)shaderVar->Value;
                ptr[0] = Position.x;
                ptr[1] = Position.y;
                ptr[2] = Position.z;
            }

            size_t offset = 0;
            foreach (ShaderVariable variable in sceneDataBuffer.PushConstantVariableList)
            {
                offset = ((size_t)offset + (size_t)variable.ByteAlignment - 1) & ~((size_t)variable.ByteAlignment - 1);
                byte* sourcePtr = (byte*)variable.Value;
                byte* destPtr = (byte*)sceneDataBuffer.PushConstantBuffer + (int)offset;
                Buffer.MemoryCopy(sourcePtr, destPtr, (nuint)variable.Size, (nuint)variable.Size);
                offset += (size_t)variable.Size;
            }
        }
    }
}