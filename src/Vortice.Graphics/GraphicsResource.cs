// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    public abstract class GraphicsResource : IDisposable
    {
        protected GraphicsResource(GraphicsDevice device)
        {
            Guard.AssertNotNull(device);

            GraphicsDevice = device;
        }

        public GraphicsDevice GraphicsDevice { get; }

        /// <summary>
        /// Finalizes an instance of the <see cref="GraphicsResource" /> class.
        /// </summary>
        ~GraphicsResource() => Dispose(disposing: false);

        /// <inheritdoc />
        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }

        /// <inheritdoc cref="Dispose()" />
        /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
        protected abstract void Dispose(bool disposing);
    }
}
