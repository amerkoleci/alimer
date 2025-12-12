// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeTexture : Texture
{
    private readonly NativeGraphicsDevice _device;

    public NativeTexture(NativeGraphicsDevice device, in TextureDescription description, TextureData* initialData)
        : base(description)
    {
        _device = device;
    }

    public override GraphicsDevice Device => _device;

    protected internal override void Destroy() => throw new NotImplementedException();
}
