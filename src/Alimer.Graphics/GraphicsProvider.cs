// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Immutable;

namespace Alimer.Graphics
{
    public abstract class GraphicsProvider : IDisposable
    {
        public const string EnableValidationSwitchName = "Alimer.Graphics.EnableValidation";
        public const string EnableGPUBasedValidationSwitchName = "Alimer.Graphics.EnableGPUBasedValidation";

        public bool EnableValidation { get; }
        public bool EnableGPUBasedValidation { get; }

        protected GraphicsProvider()
        {
            if (!AppContext.TryGetSwitch(EnableValidationSwitchName, out var validationValue))
            {
#if DEBUG
                validationValue = true;
                AppContext.SetSwitch(EnableValidationSwitchName, validationValue);
#endif
            }

            EnableValidation = validationValue;

            if (AppContext.TryGetSwitch(EnableGPUBasedValidationSwitchName, out validationValue))
            {
                EnableGPUBasedValidation = validationValue;
            }
        }

        /// <summary>
        /// Gets the <see cref="GraphicsAdapter"/> available to the provider.
        /// </summary>
        /// <exception cref="ObjectDisposedException">The provider has been disposed.</exception>
        public abstract ImmutableArray<GraphicsAdapter> GraphicsAdapters { get; }

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
