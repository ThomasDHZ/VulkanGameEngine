using GameScriptLibraryDLL.Components;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCS
{
    public static class ComponentRegistry
    {
        private static readonly Dictionary<ComponentTypeEnum, Type> _typeMap = new();

        public static void Register(ComponentTypeEnum enumValue, Type structType)
        {
            if (!structType.IsValueType && !structType.IsClass)
                throw new ArgumentException("Component must be struct or class with proper layout");

            _typeMap[enumValue] = structType;
        }

        public static Type? GetTypeFor(ComponentTypeEnum componentType)
        {
            _typeMap.TryGetValue(componentType, out var type);
            return type;
        }

        public static IEnumerable<Type> GetAllRegisteredTypes()
        {
            return _typeMap.Values;
        }

        public static IEnumerable<ComponentTypeEnum> GetAllRegisteredEnums()
        {
            return _typeMap.Keys;
        }

        public static void Initialize()
        {
            _typeMap.Clear();
            Register(ComponentTypeEnum.kInputComponent, typeof(InputComponent));
            Register(ComponentTypeEnum.kSpriteComponent, typeof(SpriteComponent));
            Register(ComponentTypeEnum.kTransform2DComponent, typeof(Transform2DComponent));
            // Register(ComponentTypeEnum.kTransform3DComponent, typeof(Transform3DComponent));
            Register(ComponentTypeEnum.kDirectionalLightComponent, typeof(DirectionalLightComponent));
            Register(ComponentTypeEnum.kPointLightComponent, typeof(PointLightComponent));
            Console.WriteLine($"ComponentRegistry initialized with {_typeMap.Count} components.");
        }
    }
}
