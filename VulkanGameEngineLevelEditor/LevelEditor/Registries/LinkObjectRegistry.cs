using System;
using System.Collections.Generic;
using VulkanGameEngineLevelEditor.GameEngine;

namespace VulkanGameEngineLevelEditor.LevelEditor.Registries
{
    public readonly struct TextureHandle
    {
        public readonly IntPtr Value;
        public TextureHandle(IntPtr value) => Value = value;

        public static implicit operator IntPtr(TextureHandle h) => h.Value;
        public static explicit operator TextureHandle(IntPtr value) => new(value);
    }

    public readonly struct MaterialHandle
    {
        public readonly IntPtr Value;
        public MaterialHandle(IntPtr value) => Value = value;

        public static implicit operator IntPtr(MaterialHandle h) => h.Value;
        public static explicit operator MaterialHandle(IntPtr value) => new(value);
    }

    public readonly struct DirectionalLightHandle
    {
        public readonly IntPtr Value;
        public DirectionalLightHandle(IntPtr value) => Value = value;

        public static implicit operator IntPtr(DirectionalLightHandle h) => h.Value;
        public static explicit operator DirectionalLightHandle(IntPtr value) => new(value);
    }

    public readonly struct PointLightHandle
    {
        public readonly IntPtr Value;
        public PointLightHandle(IntPtr value) => Value = value;

        public static implicit operator IntPtr(PointLightHandle h) => h.Value;
        public static explicit operator PointLightHandle(IntPtr value) => new(value);
    }

    public static class LinkObjectRegistry
    {
        private static readonly Dictionary<Type, Delegate> _resolvers = new();

        public static void Register<THandle, TResolved>(Func<THandle, TResolved> resolver)
            where THandle : struct
        {
            if (resolver == null) throw new ArgumentNullException(nameof(resolver));

            var key = typeof(THandle);
            if (_resolvers.ContainsKey(key))
                Console.WriteLine($"[ObjectLinkerRegistry] Overwriting resolver for {key.Name}");

            _resolvers[key] = resolver;
        }

        public static object Resolve(Type handleType, object handleValue)
        {
            if (handleType == null || handleValue == null || !_resolvers.TryGetValue(handleType, out var del))
            {
                Console.WriteLine($"[ObjectLinkerRegistry] No resolver found for handle type: {handleType?.Name}");
                return null;
            }

            try
            {
                return del.DynamicInvoke(handleValue);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ObjectLinkerRegistry] Error resolving {handleType.Name}: {ex.Message}");
                return null;
            }
        }

        public static void Initialize()
        {
            _resolvers.Clear();

            // Register your resolvers here
            Register<DirectionalLightHandle, IntPtr>(handle => LightSystem.GetDirectionalLight((uint)handle.Value));
            Register<PointLightHandle, IntPtr>(handle => LightSystem.GetPointLight((uint)handle.Value));
            // Register<TextureHandle, IntPtr>(handle => TextureSystem.GetTexture(handle));   // when ready
            // Register<MaterialHandle, IntPtr>(handle => MaterialSystem.GetMaterial(handle));

            Console.WriteLine($"[ObjectLinkerRegistry] Initialized with {_resolvers.Count} resolvers.");
        }
    }
}