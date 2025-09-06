using GlmSharp;
using System;
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

        public override void Update(ShaderPushConstant sceneDataBuffer)
        {
            mat4 transform = mat4.Translate(Position) * mat4.Rotate(VMath.DegreesToRadians(0.0f), new vec3(0, 0, 1));
            ViewMatrix = transform.Inverse;

            float Aspect = Width / Height;
            ProjectionMatrix = mat4.Ortho(-Aspect * Zoom, Aspect * Zoom, -1.0f * Zoom, 1.0f * Zoom, -10.0f, 10.0f);

            mat4 modifiedProjectionMatrix = ProjectionMatrix;
            modifiedProjectionMatrix[1, 1] *= -1;

            ViewScreenSize = new vec2((Aspect * Zoom) * 2, (1.0f * Zoom) * 2);

            var shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "CameraPosition");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* ptr = (float*)shaderVar->Value;
                ptr[0] = Position.x;
                ptr[1] = Position.y;
                ptr[2] = Position.z;
            }

            shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "ViewMatrix");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* matrixPtr = (float*)shaderVar->Value;
                for (int i = 0; i < 16; i++)
                {
                    matrixPtr[i] = ViewMatrix[i];
                }
            }

            shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "Projection");
            if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
            {
                float* matrixPtr = (float*)shaderVar->Value;
                for (int i = 0; i < 16; i++)
                {
                    matrixPtr[i] = modifiedProjectionMatrix[i];
                }
            }
        }
    }
}