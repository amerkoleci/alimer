// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class GraphicsResource : GraphicsObject
{
    protected GraphicsResource(GraphicsDevice device, string? label = default)
        : base(device, label)
    {
    }
}
