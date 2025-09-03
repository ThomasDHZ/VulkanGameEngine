using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public unsafe struct MemoryLeakPtr
{
    public void* PtrAddress;
    public size_t PtrElements;
    public bool isArray;
    public IntPtr DanglingPtrMessage;
    public IntPtr File;
    public IntPtr Line;
    public IntPtr Function;
    public IntPtr Notes;
}   