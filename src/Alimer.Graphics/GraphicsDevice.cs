// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Immutable;

namespace Alimer.Graphics
{
    public abstract class GraphicsDevice : IDisposable
    {
        private readonly GraphicsAdapter _adapter;

        /// <summary>
        /// Create new instance of the <see cref="GraphicsDevice" /> class.
        /// </summary>
        /// <param name="adapter">The physical graphics adapter.</param>
        /// <exception cref="ArgumentNullException"><paramref name="adapter" /> is <c>null</c>.</exception>
        protected GraphicsDevice(GraphicsAdapter adapter)
        {
            Guard.ThrowIfNull(adapter, nameof(adapter));

            _adapter = adapter;
        }

        /// <inheritdoc />
        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }

        /// <inheritdoc cref="Dispose()" />
        /// <param name="disposing">
        /// <c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.
        /// </param>
        protected abstract void Dispose(bool disposing);
    }
}
