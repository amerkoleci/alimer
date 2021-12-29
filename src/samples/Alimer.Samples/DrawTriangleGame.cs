// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using Alimer.Graphics;

namespace Alimer.Samples;

public sealed class DrawTriangleGame : Game
{
    public DrawTriangleGame(GameContext context)
        : base(context)
    {
    }

    protected override void Initialize()
    {
        base.Initialize();

        using (var texture = GraphicsDevice.CreateTexture(TextureDescriptor.Texture2D(TextureFormat.RGBA8UNorm, 256, 256)))
        {
        }
    }
}
