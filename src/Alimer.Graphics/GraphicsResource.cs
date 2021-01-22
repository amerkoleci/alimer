// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer.Graphics
{
    public abstract class GraphicsResource : IDisposable
    {
        protected GraphicsResource(GraphicsDevice device)
        {
            Guard.AssertNotNull(device, nameof(device));

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
