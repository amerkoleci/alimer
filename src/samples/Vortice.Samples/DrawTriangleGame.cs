// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.Engine;
using Vortice.Graphics;

namespace Vortice.Samples
{
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
}
