using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngine;
using static ComponentRegistry;

namespace VulkanGameEngineLevelEditor.LevelEditor.Registries
{
    public static class LightRegistry
    {
        private static readonly Dictionary<LightTypeEnum, Type> _typeMap = new();

        public static void Register(LightTypeEnum enumValue, Type structType)
        {
            if (!structType.IsValueType && !structType.IsClass) // usually structs for ECS
                throw new ArgumentException("Component must be struct or class with proper layout");

            _typeMap[enumValue] = structType;
        }

        public static Type? GetTypeFor(LightTypeEnum lightType)
        {
            _typeMap.TryGetValue(lightType, out var type);
            return type;
        }

        public static IEnumerable<Type> GetAllRegisteredTypes()
        {
            return _typeMap.Values;
        }

        public static IEnumerable<LightTypeEnum> GetAllRegisteredEnums()
        {
            return _typeMap.Keys;
        }

        public static void Initialize()
        {
            _typeMap.Clear();
            Register(LightTypeEnum.kDirectionalLight, typeof(DirectionalLight));
            Register(LightTypeEnum.kPointLight, typeof(PointLight));
        }
    }
}
