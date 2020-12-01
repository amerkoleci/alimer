// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    public abstract class GraphicsDevice : IDisposable
    {
        public const string EnableValidationSwitchName = "Alimer.Graphics.EnableValidation";
        public const string EnableGPUBasedValidationSwitchName = "Alimer.Graphics.EnableGPUBasedValidation";

        static GraphicsDevice()
        {
            if (!AppContext.TryGetSwitch(EnableValidationSwitchName, out bool validationValue))
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
                if (validationValue)
                {
                    EnableValidation = true;
                }
            }
        }

        public static GraphicsAdapterType AdapterPreference { get; set; } = GraphicsAdapterType.DiscreteGPU;

        public static bool EnableValidation { get; }
        public static bool EnableGPUBasedValidation { get; }

        protected GraphicsDevice()
        {
        }

        public abstract GraphicsDeviceCaps Capabilities { get; }

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
