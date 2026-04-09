using System;
using System.Reflection;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngine;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public unsafe class DynamicComponentWrapper
    {
        public uint GameObjectId { get; }
        public ComponentTypeEnum ComponentType { get; }
        public IntPtr ComponentPtr { get; private set; }
        public Type ComponentStructType { get; }

        public string DisplayName => ComponentType.ToString();

        public DynamicComponentWrapper(uint gameObjectId, ComponentTypeEnum componentType,
                                       IntPtr componentPtr, Type structType)
        {
            GameObjectId = gameObjectId;
            ComponentType = componentType;
            ComponentPtr = componentPtr;
            ComponentStructType = structType ?? throw new ArgumentNullException(nameof(structType));
        }

        public object GetMemberValue(MemberInfo member)
        {
            if (ComponentPtr == IntPtr.Zero)
                return null;

            try
            {
                object structure = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);

                Console.WriteLine($"[Debug] Reflecting on struct: {ComponentStructType.Name}, member: {member.Name} ({member.GetType().Name})");

                if (member is FieldInfo field)
                {
                    var val = field.GetValue(structure);
                    Console.WriteLine($"[Debug] Field {field.Name} = {val}");
                    return val;
                }

                if (member is PropertyInfo prop && prop.CanRead)
                {
                    var val = prop.GetValue(structure);
                    Console.WriteLine($"[Debug] Property {prop.Name} = {val}");
                    return val;
                }

                return null;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[GetMemberValue] Exception on {ComponentType}.{member?.Name}: {ex.Message}");
                return null;
            }
        }

        public void SetMemberValue(MemberInfo member, object value)
        {
            if (ComponentPtr == IntPtr.Zero || value == null)
                return;

            try
            {
                object structure = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);

                if (member is FieldInfo field)
                    field.SetValue(structure, value);
                else if (member is PropertyInfo prop && prop.CanWrite)
                    prop.SetValue(structure, value);

                Marshal.StructureToPtr(structure, ComponentPtr, false);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[SetMemberValue] Failed on {ComponentType}.{member?.Name}: {ex.Message}");
            }
        }
    }
}