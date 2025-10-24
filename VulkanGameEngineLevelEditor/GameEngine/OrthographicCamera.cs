using GlmSharp;
using System;
using System.Numerics;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngine.Systems;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using vec2 = GlmSharp.vec2;

public unsafe class OrthographicCamera : Camera
{

    public OrthographicCamera()
    {
    }

    public OrthographicCamera(float width, float height)
    {
        Initialize(width, height, new vec3(0.0f));
    }

    public OrthographicCamera(vec2 viewScreenSize)
    {
        Initialize(viewScreenSize.x, viewScreenSize.y, new vec3(0.0f));
    }

    public OrthographicCamera(vec2 viewScreenSize, vec3 position)
    {
        Initialize(viewScreenSize.x, viewScreenSize.y, position);
    }

    private void Initialize(float width, float height, vec3 position)
    {
        Width = width;
        Height = height;
        AspectRatio = width / height;
        Zoom = 1.0f;

        Position = new vec3(position);
        ViewScreenSize = new vec2(width, height);
        ProjectionMatrix = mat4.Ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, -1.0f, 1.0f);
        ViewMatrix = mat4.Identity;
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

        var shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "CameraPosition");
        if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
        {
            float* ptr = (float*)shaderVar->Value;
            ptr[0] = Position.x;
            ptr[1] = Position.y;
            ptr[2] = Position.z;
        }

        shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "ViewMatrix");
        if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
        {
            float* matrixPtr = (float*)shaderVar->Value;
            for (int i = 0; i < 16; i++)
            {
                matrixPtr[i] = ViewMatrix[i];
            }
        }

        shaderVar = ShaderSystem.SearchGlobalShaderConstantVar(sceneDataBuffer, "Projection");
        if (shaderVar != null && (IntPtr)shaderVar->Value != IntPtr.Zero)
        {
            float* matrixPtr = (float*)shaderVar->Value;
            for (int i = 0; i < 16; i++)
            {
                matrixPtr[i] = modifiedProjectionMatrix[i];
            }
        }
    }

    public override void UpdateKeyboard(float deltaTime)
    {
    }

    public override void UpdateMouse()
    {
    }
}