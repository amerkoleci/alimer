// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics;

[StructLayout(LayoutKind.Explicit)]
public readonly struct GraphicsNativeHandle
{
    [FieldOffset(0)]
    public readonly nint Handle;

    [FieldOffset(0)]
    public readonly ulong UlongHandle;

    public readonly bool IsValid => Handle != 0;

    public static GraphicsNativeHandle InvalidHandle => new(0);

    public GraphicsNativeHandle(nint handle)
    {
        Handle = handle;
    }

    public GraphicsNativeHandle(ulong handle)
    {
        UlongHandle = handle;
    }
}
