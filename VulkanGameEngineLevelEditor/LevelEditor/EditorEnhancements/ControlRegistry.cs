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
        private static readonly Dictionary<Type, Func<object, MemberInfo, int, bool, ObjectPanelView, Control>> _controlCreators = new()
        {
            { typeof(string), (obj, member, height, readOnly, parentPanel) => new TypeOfStringForm(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(float), (obj, member, height, readOnly, parentPanel) => new TypeOfFloat(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(int), (obj, member, height, readOnly, parentPanel) => new TypeOfIntForm(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(uint), (obj, member, height, readOnly, parentPanel) => new TypeOfUintForm(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(bool), (obj, member, height, readOnly, parentPanel) => new TypeOfBool(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(Guid), (obj, member, height, readOnly, parentPanel) => new TypeOfGuidForm(parentPanel, obj, member, height, readOnly).CreateControl() },
            { typeof(vec2), (obj, member, height, readOnly, parentPanel) => new TypeOfVec2Form(parentPanel, obj, member, height, readOnly).CreateControl() }
            // Add other types as needed
        };

        public static Control CreateControl(ObjectPanelView parentPanel, Type type, object obj, MemberInfo member, int height, bool readOnly)
        {
            try
            {
                if (_controlCreators.TryGetValue(type, out var creator))
                {
                    return creator(obj, member, height, readOnly, parentPanel);
                }
                if (type.BaseType == typeof(Enum))
                {
                    return new TypeOfEnum(parentPanel,obj, member, height, readOnly).CreateControl();
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