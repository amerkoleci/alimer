// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics.Null
{
    public class TextureNull : Texture
    {
        public TextureNull(GraphicsDeviceNull device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
        }
    }
}
