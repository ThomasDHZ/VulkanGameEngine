using System;

namespace VulkanGameEngineLevelEditor.LevelEditor.Attributes
{
    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
    public class LinkObjectAttribute : Attribute
    {
        public Type HandleType { get; }

        public LinkObjectAttribute(Type handleType)
        {
            HandleType = handleType ?? throw new ArgumentNullException(nameof(handleType));
        }
    }
}