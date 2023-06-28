// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class BindGroup : GraphicsObject
{
    protected BindGroup(GraphicsDevice device, in BindGroupDescription description)
        : base(device, description.Label)
    {
    }
}
