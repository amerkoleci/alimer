// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum GraphicsHeapProperty
{
    None = 0,
    DeviceLocal = 1 << 0,
    HostVisible = 1 << 1,
    HostCoherent = 1 << 2,
    HostUncached = 1 << 3,
    HostCached = 1 << 4
}

public struct GraphicsMemoryHeapInfo
{
    public GraphicsHeapProperty Properties;
    public ulong Size;
}
