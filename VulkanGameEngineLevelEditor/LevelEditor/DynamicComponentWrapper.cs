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
        public IntPtr ComponentPtr { get; }
        public Type ComponentStructType { get; }

        public string DisplayName => ComponentType.ToString()
            .Replace("k", "").Replace("Component", "");

        public DynamicComponentWrapper(uint gameObjectId, ComponentTypeEnum componentType,
                                       IntPtr componentPtr, Type structType)
        {
            GameObjectId = gameObjectId;
            ComponentType = componentType;
            ComponentPtr = componentPtr;
            ComponentStructType = structType ?? throw new ArgumentNullException(nameof(structType));
        }

        /// <summary>
        /// DIRECT reference to the original native memory. 
        /// Any change here immediately affects C++ side (zero copy).
        /// </summary>
        public ref T GetComponentRef<T>() where T : struct
        {
            if (typeof(T) != ComponentStructType)
                throw new InvalidOperationException($"Type mismatch: Expected {ComponentStructType.Name}, got {typeof(T).Name}");

            if (ComponentPtr == IntPtr.Zero)
                return ref Unsafe.NullRef<T>();

            return ref Unsafe.AsRef<T>(ComponentPtr.ToPointer());
        }

        /// <summary>
        /// Get value of a member (field or property)
        /// </summary>
        public object? GetMemberValue(MemberInfo member)
        {
            if (ComponentPtr == IntPtr.Zero) return null;

            try
            {
                if (member is FieldInfo field)
                {
                    // Prefer direct field access when possible
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

        /// <summary>
        /// Set value using direct ref when possible (much more reliable)
        /// </summary>
        public void SetMemberValue(MemberInfo member, object? value)
        {
            if (ComponentPtr == IntPtr.Zero || value == null) return;

            try
            {
                if (member is FieldInfo field)
                {
                    // For public fields this is reliable
                    object boxed = Marshal.PtrToStructure(ComponentPtr, ComponentStructType);
                    field.SetValue(boxed, value);
                    Marshal.StructureToPtr(boxed, ComponentPtr, false);
                }
                else if (member is PropertyInfo prop && prop.CanWrite)
                {
                    // For properties we still need the round-trip, but it's safer now
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