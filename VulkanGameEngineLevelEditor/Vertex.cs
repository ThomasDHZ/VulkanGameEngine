﻿using GlmSharp;
using Silk.NET.Vulkan;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vertex2D
    {
        public vec2 Position;
        public vec2 UV;
        public vec4 Color;

        public Vertex2D(vec2 position, vec2 uv, vec4 color)
        {
            Position = position;
            UV = uv;
            Color = color;
        }

        public static List<VertexInputBindingDescription> GetBindingDescriptions()
        {
            var bindingDescriptions = new List<VertexInputBindingDescription>
            {
                new VertexInputBindingDescription
                {
                    Binding = 0,
                    Stride = (uint)Marshal.SizeOf(typeof(Vertex2D)),
                    InputRate = VertexInputRate.Vertex
                }
            };

            return bindingDescriptions;
        }

        public static List<VertexInputAttributeDescription> GetAttributeDescriptions()
        {
            var attributeDescriptions = new List<VertexInputAttributeDescription>();

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 0,
                Format = Format.R32G32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex2D), nameof(Position)).ToInt32()
            });

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 1,
                Format = Format.R32G32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex2D), nameof(UV)).ToInt32()
            });

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 2,
                Format = Format.R32G32B32A32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex2D), nameof(Color)).ToInt32()
            });

            return attributeDescriptions;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vertex3D
    {
        public vec3 Position;
        public vec3 Color;
        public vec2 UV;

        public Vertex3D(vec3 position, vec3 color, vec2 uv)
        {
            Position = position;
            Color = color;
            UV = uv;

        }

        public static List<VertexInputBindingDescription> GetBindingDescriptions()
        {
            var bindingDescriptions = new List<VertexInputBindingDescription>
            {
                new VertexInputBindingDescription
                {
                    Binding = 0,
                    Stride = (uint)Marshal.SizeOf(typeof(Vertex3D)),
                    InputRate = VertexInputRate.Vertex
                }
            };

            return bindingDescriptions;
        }

        public static List<VertexInputAttributeDescription> GetAttributeDescriptions()
        {
            var attributeDescriptions = new List<VertexInputAttributeDescription>();

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 0,
                Format = Format.R32G32B32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex3D), nameof(Position)).ToInt32()
            });

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 1,
                Format = Format.R32G32B32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex3D), nameof(Color)).ToInt32()
            });

            attributeDescriptions.Add(new VertexInputAttributeDescription
            {
                Binding = 0,
                Location = 2,
                Format = Format.R32G32Sfloat,
                Offset = (uint)Marshal.OffsetOf(typeof(Vertex3D), nameof(UV)).ToInt32()
            });

            return attributeDescriptions;
        }
    }
}