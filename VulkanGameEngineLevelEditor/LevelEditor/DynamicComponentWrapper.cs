using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngine;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public unsafe class DynamicComponentWrapper
    {
        public uint GameObjectId { get; }
        public ComponentTypeEnum ComponentType { get; }
        public LightTypeEnum LightType { get; }
        public IntPtr ComponentPtr { get; }
        public Type ComponentStructType { get; }

        public string DisplayName => ComponentType.ToString().Replace("k", "").Replace("Component", "");

        public DynamicComponentWrapper(uint gameObjectId, ComponentTypeEnum componentType, IntPtr componentPtr, Type structType)
        {
            GameObjectId = gameObjectId;
            ComponentType = componentType;
            ComponentPtr = componentPtr;
            ComponentStructType = structType ?? throw new ArgumentNullException(nameof(structType));
        }

        public DynamicComponentWrapper(uint gameObjectId, LightTypeEnum lightType, IntPtr componentPtr, Type structType)
        {
            GameObjectId = gameObjectId;
            LightType = lightType;
            ComponentPtr = componentPtr;
            ComponentStructType = structType ?? throw new ArgumentNullException(nameof(structType));
        }

        public object? GetMemberValue(MemberInfo member)
        {
            if (ComponentPtr == IntPtr.Zero) return null;

            try
            {
                if (member is FieldInfo field)
                {
                    object boxed = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);
                    return field.GetValue(boxed);
                }
                else if (member is PropertyInfo prop && prop.CanRead)
                {
                    object boxed = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);
                    return prop.GetValue(boxed);
                }
                return null;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[GetMemberValue] Error on {ComponentType}.{member.Name}: {ex}");
                return null;
            }
        }

        public void SetMemberValue(MemberInfo member, object? value)
        {
            if (ComponentPtr == IntPtr.Zero || value == null) return;

            try
            {
                if (member is FieldInfo field)
                {
                    object boxed = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);
                    field.SetValue(boxed, value);
                    Marshal.StructureToPtr(boxed, ComponentPtr, false);
                }
                else if (member is PropertyInfo prop && prop.CanWrite)
                {
                    object boxed = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);
                    prop.SetValue(boxed, value);
                    Marshal.StructureToPtr(boxed, ComponentPtr, false);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[SetMemberValue] FAILED {ComponentType}.{member.Name}: {ex.Message}");
            }
        }
    }
}