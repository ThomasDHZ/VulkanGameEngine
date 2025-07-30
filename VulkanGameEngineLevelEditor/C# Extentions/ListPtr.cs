using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace VulkanGameEngineLevelEditor
{
    public unsafe class ListPtr<T> : IEnumerable<T>, IDisposable where T : unmanaged
    {
        private T* _ptr;
        private T*[] _debugList; //Just here to make debugging easier.
        private size_t _count = 0;
        private size_t _capacity = 0;
        private bool _disposed;
        private bool _ptrUpdatedExternally;

        public size_t Count => _count;
        public uint UCount => (uint)_count;
        public T* Ptr
        {
            get
            {
                if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));
                _ptrUpdatedExternally = true;
                return _ptr;
            }
        }

        public ListPtr()
        {
            _ptr = null;
            _count = 0;
            _capacity = 0;
            _debugList = new T*[_capacity];
            _disposed = false;
            _ptrUpdatedExternally = false;

            AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
        }

        public ListPtr(size_t size)
        {
            if (size <= 0) throw new ArgumentException("Size must be greater than 0.");

            _count = size;
            _capacity = size;
            _debugList = new T*[_capacity];
            _ptr = (T*)Marshal.AllocHGlobal(sizeof(T) * (int)_capacity);

            ClearMemory((byte*)_ptr, sizeof(T) * (int)_capacity);
            UpdateList();

            AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
        }

        public ListPtr(T* ptr, size_t size)
        {
            if (size <= 0) throw new ArgumentException("Size must be greater than 0.");

            _count = size;
            _capacity = size;
            _ptr = (T*)Marshal.AllocHGlobal(sizeof(T) * (int)_capacity);
            _debugList = new T*[_capacity];

            for (size_t x = 0; x < size; x++)
            {
                _ptr[x] = ptr[x];
            }

            UpdateList();
            AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
        }

        public ListPtr(List<T> list)
        {
            if (list.Count <= 0) return;

            _count = list.Count;
            _capacity = list.Capacity;
            _debugList = new T*[_capacity];
            _ptr = (T*)Marshal.AllocHGlobal(sizeof(T) * (int)_capacity);

            ClearMemory((byte*)_ptr, sizeof(T) * (int)_capacity);
            for (int x = 0; x < _count; x++)
            {
                _ptr[x] = list[x];
            }

            UpdateList();

            AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
        }

        ~ListPtr()
        {
            if (!_disposed)
            {
                Console.WriteLine($"Warning: ListPtr<{typeof(T).Name}> was not disposed properly.");
                Dispose();     
            }
        }

        public T this[int index]
        {
            get
            {
                if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));
                if (index >= _count) throw new IndexOutOfRangeException("Index out of bounds");
                if (_ptrUpdatedExternally) UpdateList();
                return _ptr[index];
            }
            set
            {
                if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));
                if (index >= _count) throw new IndexOutOfRangeException("Index out of bounds");
                _ptr[index] = value;
                UpdateList();
            }
        }

        public void Add(T item)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));

            if (_count >= _capacity)
            {
                if (_capacity == 0)
                {
                    _capacity = 1;
                }
                else
                {
                    _capacity *= 2;
                }

                int totalSize = sizeof(T) * (int)_capacity;
                T* newPtr = (T*)Marshal.AllocHGlobal(totalSize);

                System.Buffer.MemoryCopy(_ptr, newPtr, sizeof(T) * _count, sizeof(T) * _count);
                Marshal.FreeHGlobal((IntPtr)_ptr);
                _ptr = newPtr;

                T*[] newList = new T*[_capacity];
                Array.Copy(_debugList, newList, _count);
                _debugList = newList;

                UpdateList();
            }

            _ptr[_count] = item;
            _debugList[_count] = &_ptr[_count];
            _count++;
        }

        public bool Remove(T item)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));
            for (uint x = 0; x < _count; x++)
            {
                if (EqualityComparer<T>.Default.Equals(_ptr[x], item))
                {
                    if (x < _count - 1)
                    {
                        int moveSize = sizeof(T) * (int)(_count - (x - 1));
                        System.Buffer.MemoryCopy(_ptr + (x + 1), _ptr + x, moveSize, moveSize);
                    }
                    _count--;
                    ClearMemory((byte*)(_ptr + _count), sizeof(T));
                    UpdateList();
                    return true;
                }
            }
            return false;
        }

        public bool Remove(uint index)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));
            if (index >= _count) throw new IndexOutOfRangeException("Index out of bounds");

            if (index < _count - 1)
            {
                int moveSize = sizeof(T) * (int)(_count - (index - 1));
                System.Buffer.MemoryCopy(_ptr + (index + 1), _ptr + index, moveSize, moveSize);
            }
            _count--;
            ClearMemory((byte*)(_ptr + _count), sizeof(T));
            UpdateList();
            return true;
        }

        public void Clear()
        {
            ClearMemory((byte*)_ptr, sizeof(T) * (int)_capacity);
            _count = 0;
            _capacity = 0;
            _debugList = new T*[0];
            UpdateList();
        }

        private void UpdateList()
        {
            if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));

            if (_debugList.Length < _count)
            {
                _debugList = new T*[_capacity];
            }

            for (uint x = 0; x < _count; x++)
            {
                _debugList[x] = &_ptr[x];
            }
            _ptrUpdatedExternally = false;
        }

        private void ClearMemory(byte* ptr, long size)
        {
            if (ptr == null || size <= 0) return;
            for (int x = 0; x < size; x++)
            {
                ptr[x] = 0;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (_disposed)
            {
                Console.WriteLine("ListPtr already disposed.");
                return;
            }

            if (_ptr != null)
            {
                Marshal.FreeHGlobal((IntPtr)_ptr);
                _ptr = null;
            }
            _disposed = true;

            if (disposing)
            {
                AppDomain.CurrentDomain.ProcessExit -= OnProcessExit;
            }
        }

        private void OnProcessExit(object sender, EventArgs e)
        {
            if (!_disposed)
            {
                Dispose(true);
            }
        }

        public static bool operator ==(ListPtr<T> l1, ListPtr<T> l2)
        {
            if (ReferenceEquals(l1, l2)) return true;
            if (l1 is null || l2 is null) return false;
            return l1.Ptr == l2.Ptr;
        }

        public static bool operator !=(ListPtr<T> l1, ListPtr<T> l2)
        {
            if (ReferenceEquals(l1, l2)) return true;
            if (l1 is null || l2 is null) return false;
            return l1.Ptr != l2.Ptr;
        }

        public IEnumerator<T> GetEnumerator()
        {
            if (_disposed) throw new ObjectDisposedException(nameof(ListPtr<T>));

            List<T> list = new List<T>((int)_count);
            for (uint x = 0; x < _count; x++)
            {
                list.Add(_ptr[x]);
            }

            return list.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public List<T> ToList()
        {
            List<T> list = new List<T>();
            for (int x = 0; x < _count; x++)
            {
                if (_ptr != null)
                {
                    list.Add(_ptr[x]);
                }
            }
            return list;
        }
    }
}
