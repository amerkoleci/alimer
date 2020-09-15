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
                if (validationValue)
                {
                    EnableValidation = true;
                }
            }
        }

        public static bool EnableValidation { get; }
        public static bool EnableGPUBasedValidation { get; }

        /// <summary>
        /// Get the device capabilities.
        /// </summary>
        public abstract GraphicsDeviceCaps Capabilities { get; }

        public BackendType BackendType => Capabilities.BackendType;

        protected GraphicsDevice()
        {
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

        public static GraphicsDevice? CreateSystemDefault(
            BackendType preferredBackendType = BackendType.Count,
            GraphicsAdapterPreference adapterPreference = GraphicsAdapterPreference.Default)
        {
            if (preferredBackendType == BackendType.Count)
            {
                preferredBackendType = GetDefaultPlatformBackend();
            }

            switch (preferredBackendType)
            {
#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    return new D3D12.D3D12GraphicsDevice(adapterPreference);
#endif
#if !EXCLUDE_D3D11_BACKEND
                case BackendType.Direct3D11:
                    return null;
#endif
                default:
                    return null;
            }
        }

        public static bool IsBackendSupported(BackendType backend)
        {
            if (backend == BackendType.Count)
            {
                backend = GetDefaultPlatformBackend();
            }

            switch (backend)
            {
                case BackendType.Null:
                    return true;
#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    return D3D12.D3D12GraphicsDevice.IsSupported();
#endif
#if !EXCLUDE_D3D11_BACKEND
                case BackendType.Direct3D11:
                    return false;
#endif
                default:
                    return false;
            }
        }

        /// <summary>
        /// Gets the best supported <see cref="BackendType"/> on the current platform.
        /// </summary>
        /// <returns></returns>
        public static BackendType GetDefaultPlatformBackend()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                if (IsBackendSupported(BackendType.Direct3D12))
                {
                    return BackendType.Direct3D12;
                }

                if (IsBackendSupported(BackendType.Vulkan))
                {
                    return BackendType.Vulkan;
                }

                return BackendType.Direct3D11;
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return BackendType.Metal;
            }

            if (IsBackendSupported(BackendType.Vulkan))
            {
                return BackendType.Vulkan;
            }

            return BackendType.Null;
        }
    }
}
