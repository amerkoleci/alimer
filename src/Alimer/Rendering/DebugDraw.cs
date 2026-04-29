// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed unsafe class DebugDraw : DisposableObject
{
    public DebugDraw(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
    }

    public GraphicsDevice Device { get; }
}
