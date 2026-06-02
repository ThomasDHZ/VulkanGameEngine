using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameScriptLibraryDLL.Components
{
    public static class ComponentRegistery
    {
        private static readonly Dictionary<ComponentTypeEnum, Action<uint, object>> _creators = new();

        public static void Register<T>(ComponentTypeEnum type, Action<uint, T> creator) where T : unmanaged
        {
            _creators[type] = (id, data) => creator(id, (T)data);
        }

        public static void Create(uint gameObjectId, ComponentTypeEnum type)
        {
            if (_creators.TryGetValue(type, out var creator))
            {
                creator(gameObjectId);
            }
        }

        static ComponentRegistery()
        {
            ComponentRegistery.Register<Transform2DComponent>(ComponentTypeEnum.kTransform2DComponent, (id, data) => Component.CreateGameObjectComponent(id, ComponentTypeEnum.kTransform2DComponent, ref data));
            ComponentRegistery.Register<SpriteComponent>(ComponentTypeEnum.kSpriteComponent, (id, data) => Component.CreateGameObjectComponent(id, ComponentTypeEnum.kSpriteComponent, ref data));
            ComponentRegistery.Register<DirectionalLightComponent>(ComponentTypeEnum.kDirectionalLightComponent, (id, data) => Component.CreateGameObjectComponent(id, ComponentTypeEnum.kDirectionalLightComponent, ref data));
            ComponentRegistery.Register<PointLightComponent>(ComponentTypeEnum.kPointLightComponent, (id, data) => Component.CreateGameObjectComponent(id, ComponentTypeEnum.kPointLightComponent, ref data));
        }
    }
}
