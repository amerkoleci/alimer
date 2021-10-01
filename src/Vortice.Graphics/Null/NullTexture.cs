// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics.Null
{
    public class NullTexture : Texture
    {
        public NullTexture(NullGraphicsDevice device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
        }
    }
}
