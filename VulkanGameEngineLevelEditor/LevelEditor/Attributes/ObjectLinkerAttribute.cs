using System;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
    public class ObjectLinkerAttribute : Attribute
    {
        public Type HandleType { get; }

        public ObjectLinkerAttribute(Type handleType)
        {
            HandleType = handleType ?? throw new ArgumentNullException(nameof(handleType));
        }
    }
}