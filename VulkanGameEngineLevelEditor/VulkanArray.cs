//using System;
//using System.Collections;
//using System.Collections.Generic;
//using System.Runtime.CompilerServices;
//using System.Runtime.InteropServices;

//namespace VulkanGameEngineLevelEditor
//{
//    /// <summary>
//    /// Very lightweight wrapper for native arrays of Vulkan handles received from C++.
//    /// Copies data once — does NOT take ownership of C++ memory.
//    /// Does NOT grow/shrink — Vulkan arrays are recreated on swapchain rebuild anyway.
//    /// </summary>
//    public unsafe sealed class VulkanArray<T> : IEnumerable<T>, IDisposable
//        where T : struct
//    {
//        private T* _ptr;          // ← FIXED: no 'unsafe' keyword here
//        private int _length;
//        private bool _disposed;

//        public int Length => _length;
//        public uint ULength => (uint)_length;

//        public T* Ptr
//        {
//            get
//            {
//                if (_disposed) throw new ObjectDisposedException(nameof(VulkanArray<T>));
//                return _ptr;
//            }
//        }

//        public VulkanArray(ReadOnlySpan<T> data)
//        {
//            if (data.IsEmpty)
//            {
//                _ptr = null;
//                _length = 0;
//                return;
//            }

//            _length = data.Length;
//            int byteSize = sizeof(T) * _length;
//            _ptr = (T*)NativeMemory.Alloc((nuint)byteSize);

//            data.CopyTo(new Span<T>(_ptr, _length));
//        }

//        public VulkanArray(T* source, int count)
//        {
//            if (count <= 0 || source == null)
//            {
//                _ptr = null;
//                _length = 0;
//                return;
//            }

//            _length = count;
//            int byteSize = sizeof(T) * count;
//            _ptr = (T*)NativeMemory.Alloc((nuint)byteSize);

//            new Span<T>(source, count).CopyTo(new Span<T>(_ptr, count));
//        }

//        public T this[int index]
//        {
//            get
//            {
//                if (_disposed) throw new ObjectDisposedException(nameof(VulkanArray<T>));
//                if ((uint)index >= (uint)_length) throw new IndexOutOfRangeException();
//                return _ptr[index];
//            }
//        }

//        public Span<T> AsSpan() => _disposed ? default : new Span<T>(_ptr, _length);
//        public ReadOnlySpan<T> AsReadOnlySpan() => AsSpan();

//        public void Dispose()
//        {
//            if (_disposed) return;
//            if (_ptr != null)
//            {
//                NativeMemory.Free(_ptr);
//                _ptr = null;
//            }
//            _disposed = true;
//            GC.SuppressFinalize(this);
//        }

//        ~VulkanArray()
//        {
//            Dispose();
//        }

//        public IEnumerator<T> GetEnumerator()
//        {
//            if (_disposed) throw new ObjectDisposedException(nameof(VulkanArray<T>));
//            for (int i = 0; i < _length; i++)
//                yield return _ptr[i];
//        }

//        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

//        public T[] ToArray()
//        {
//            if (_disposed || _length == 0) return Array.Empty<T>();
//            var arr = new T[_length];
//            AsSpan().CopyTo(arr);
//            return arr;
//        }
//    }
//}