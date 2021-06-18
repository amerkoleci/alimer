// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Diagnostics;

namespace Vortice.Graphics
{
    public abstract class Texture : GraphicsResource
    {
        protected Texture(GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device)
        {
        }
    }
}
