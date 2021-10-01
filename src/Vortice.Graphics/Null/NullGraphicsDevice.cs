// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics.Null
{
    public sealed  class NullGraphicsDevice : GraphicsDevice
    {
        public NullGraphicsDevice()
            : base(GraphicsBackend.Null, physicalDevice: null)
        {
            Capabilities = default;
        }

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities { get; }

        /// <summary>
        /// Finalizes an instance of the <see cref="NullGraphicsDevice" /> class.
        /// </summary>
        ~NullGraphicsDevice() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
        }

        /// <inheritdoc />
        protected override SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor) => throw new System.NotImplementedException();

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new NullTexture(this, descriptor);
    }
}
