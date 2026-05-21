// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly struct GraphicsNativeHandle(GraphicsNativeHandleType type, nint handle)
{
    public readonly GraphicsNativeHandleType Type = type;
    public readonly nint Handle = handle;

    public readonly bool IsValid => Type != GraphicsNativeHandleType.Unknown && Handle != 0;

    public static GraphicsNativeHandle Invalid => new(GraphicsNativeHandleType.Unknown, 0);
}
