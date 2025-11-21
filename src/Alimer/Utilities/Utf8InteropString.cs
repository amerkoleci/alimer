// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using System.Text;

namespace Alimer;

public unsafe ref struct Utf8InteropString
{
    public nint Handle;

    public Utf8InteropString(string str)
    {
        if (str != null)
        {
            uint byteCount = (uint)Encoding.UTF8.GetByteCount(str);
            byte* ptr = (byte*)NativeMemory.Alloc(byteCount + 1);
            fixed (char* chars = str)
            {
                Encoding.UTF8.GetBytes(chars, str.Length, ptr, (int)byteCount);
            }
            ptr[byteCount] = 0;
            Handle = (nint)ptr;
        }
    }

    public void Free()
    {
        NativeMemory.Free((void*)Handle);
        Handle = 0;
    }
}
