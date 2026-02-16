// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class AccelerationStructure : GraphicsObject, IGraphicsBindableResource
{
    protected AccelerationStructure(in AccelerationStructureDescriptor descriptor)
        : base(descriptor.Label)
    {
    }
}
