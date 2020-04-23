// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Immutable;

namespace Alimer.Graphics
{
    public abstract class GraphicsAdapter : IDisposable
    {
        private readonly GraphicsProvider _provider;

        /// <summary>
        /// Create new instance of the <see cref="GraphicsAdapter" /> class.
        /// </summary>
        /// <param name="provider">The graphics provider which enumerated the adapter.</param>
        /// <exception cref="ArgumentNullException"><paramref name="provider" /> is <c>null</c>.</exception>
        protected GraphicsAdapter(GraphicsProvider provider)
        {
            Guard.ThrowIfNull(provider, nameof(provider));
            _provider = provider;
        }

        public abstract int VendorId { get; }
        public abstract int DeviceId { get; }
        public abstract string Name { get; }

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
