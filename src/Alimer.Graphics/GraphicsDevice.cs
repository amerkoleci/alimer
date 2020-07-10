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
        public BackendType BackendType { get; }
        public abstract string Name { get; }
        public abstract bool IsLowPower { get; }
        public abstract int VendorId { get; }
        public abstract int DeviceId { get; }

        /// <summary>
        /// Create new instance of the <see cref="GraphicsDevice" /> class.
        /// </summary>
        protected GraphicsDevice(BackendType backendType)
        {
            Guard.Assert(backendType != BackendType.Default, "Invalid backend type");

            BackendType = backendType;
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

        public static GraphicsDevice? CreateSystemDefault(BackendType preferredBackendType = BackendType.Default)
        {
            if (preferredBackendType == BackendType.Default)
            {
                preferredBackendType = GetDefaultPlatformBackend();
            }

            switch (preferredBackendType)
            {
#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    return new D3D12.D3D12GraphicsDevice();
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
            if (backend == BackendType.Default)
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


        private static BackendType GetDefaultPlatformBackend()
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

                if (IsBackendSupported(BackendType.Direct3D11))
                {
                    return BackendType.Direct3D11;
                }

                return BackendType.OpenGL;
            }

            if (IsBackendSupported(BackendType.Vulkan))
            {
                return BackendType.Vulkan;
            }

            return BackendType.OpenGL;
        }
    }
}
