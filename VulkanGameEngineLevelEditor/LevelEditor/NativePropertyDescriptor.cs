using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.LevelEditor
{
    public class NativePropertyDescriptor : PropertyDescriptor
    {
        private readonly PropertyDescriptor _inner;
        private readonly IntPtr _nativePtr;

        public NativePropertyDescriptor(PropertyDescriptor inner, IntPtr nativePtr)
            : base(inner.Name, null)
        {
            _inner = inner ?? throw new ArgumentNullException(nameof(inner));
            _nativePtr = nativePtr;
        }

        public override Type ComponentType => _inner.ComponentType;
        public override bool IsReadOnly => _inner.IsReadOnly;
        public override Type PropertyType => _inner.PropertyType;

        public override bool CanResetValue(object component) => _inner.CanResetValue(component);
        public override void ResetValue(object component) => _inner.ResetValue(component);
        public override bool ShouldSerializeValue(object component) => _inner.ShouldSerializeValue(component);

        public override object GetValue(object component)
        {
            if (_nativePtr == IntPtr.Zero)
            {
                return _inner.PropertyType.IsValueType ? Activator.CreateInstance(_inner.PropertyType) : null;
            }

            object fullStruct = Marshal.PtrToStructure(_nativePtr, _inner.ComponentType);
            return _inner.GetValue(fullStruct);
        }

        public override void SetValue(object component, object value)
        {
            if (_nativePtr == IntPtr.Zero)
            {
                return;
            }

            object fullStruct = Marshal.PtrToStructure(_nativePtr, _inner.ComponentType);
            _inner.SetValue(fullStruct, value);
            Marshal.StructureToPtr(fullStruct, _nativePtr, false);
        }
    }
}
