using GlmSharp;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Windows.Forms;
using VulkanGameEngineLevelEditor.LevelEditor.ControlSubForms;
using VulkanGameEngineLevelEditor.LevelEditor.Dialog;

namespace VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements
{
    public static class ControlRegistry
    {
        private static readonly Dictionary<Type, Func<object, MemberInfo, int, bool, Control>> _controlCreators = new()
        {
            { typeof(string), (obj, member, height, readOnly) => new TypeOfStringForm(obj, member, height, readOnly).CreateControl() },
            { typeof(float), (obj, member, height, readOnly) => new TypeOfFloat(obj, member, height, readOnly).CreateControl() },
            { typeof(int), (obj, member, height, readOnly) => new TypeOfIntForm(obj, member, height, readOnly).CreateControl() },
            { typeof(uint), (obj, member, height, readOnly) => new TypeOfUintForm(obj, member, height, readOnly).CreateControl() },
            { typeof(bool), (obj, member, height, readOnly) => new TypeOfBool(obj, member, height, readOnly).CreateControl() },
            { typeof(Guid), (obj, member, height, readOnly) => new TypeOfGuidForm(obj, member, height, readOnly).CreateControl() },
            { typeof(vec2), (obj, member, height, readOnly) => new TypeOfVec2Form(obj, member, height, readOnly).CreateControl() }
        };

        public static Control CreateControl(Type type, object obj, MemberInfo member, int height, bool readOnly)
        {
            try
            {
                if (_controlCreators.TryGetValue(type, out var creator))
                {
                    return creator(obj, member, height, readOnly);
                }
                if (type.BaseType == typeof(Enum))
                {
                    return new TypeOfEnum(obj, member, height, readOnly).CreateControl();
                }
                return null;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error creating control for type {type}: {ex.Message}");
                return null;
            }
        }
    }
}