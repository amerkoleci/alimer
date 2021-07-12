// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics.Null
{
    public sealed  class GraphicsDeviceNull : GraphicsDevice
    {
        private readonly GraphicsDeviceCaps _caps;
        public GraphicsDeviceNull()
        {
            _caps = new GraphicsDeviceCaps()
            {
                BackendType = GraphicsBackend.Null,
            };
        }

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities => _caps;

        /// <summary>
        /// Finalizes an instance of the <see cref="GraphicsDeviceNull" /> class.
        /// </summary>
        ~GraphicsDeviceNull() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
        }

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new TextureNull(this, descriptor);
    }
}
