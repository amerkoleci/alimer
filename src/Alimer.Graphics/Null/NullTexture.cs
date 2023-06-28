// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Null;

internal class NullTexture : Texture
{
    public NullTexture(NullGraphicsDevice device, in TextureDescriptor descriptor)
        : base(device, descriptor)
    {
        
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="NullTexture" /> class.
    /// </summary>
    ~NullTexture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
