// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Vortice.Graphics.Null
{
    /// <summary>
    /// Null (headless) graphics device implementation.
    /// </summary>
    public unsafe class NullGraphicsDevice : GraphicsDevice
    {
        public NullGraphicsDevice()
            : base()
        {
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="NullGraphicsDevice" /> class.
        /// </summary>
        ~NullGraphicsDevice() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
        }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => default;

        protected override SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor) => throw new NotImplementedException();
    }
}
