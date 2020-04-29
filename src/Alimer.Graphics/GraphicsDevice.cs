// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;

#if !EXCLUDE_D3D12_BACKEND
using Alimer.Graphics.D3D12;
#endif

#if !EXCLUDE_D3D11_BACKEND
using Alimer.Graphics.D3D11;
#endif

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

        /// <summary>
        /// Create new instance of the <see cref="GraphicsDevice" /> class.
        /// </summary>
        protected GraphicsDevice()
        {

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
                    return D3D12Instance.IsSupported();
#endif
#if !EXCLUDE_D3D11_BACKEND
                case BackendType.Direct3D11:
                    return D3D11Instance.IsSupported();
#endif
                default:
                    return false;
            }
        }

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

        public static GraphicsDevice? Create(in DeviceDescriptor descriptor)
        {
            BackendType backend = descriptor.PreferredBackend;
            if (backend == BackendType.Default)
            {
                backend = GetDefaultPlatformBackend();
            }

            switch (backend)
            {
                case BackendType.Null:
                    break;

#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    if (D3D12Instance.IsSupported())
                    {
                        return new D3D12GraphicsDevice(descriptor);
                    }

                    return null;
#endif

#if !EXCLUDE_D3D11_BACKEND
                case BackendType.Direct3D11:
                    if (D3D11Instance.IsSupported())
                    {
                        return new D3D11GraphicsDevice(descriptor);
                    }

                    return null;
#endif

            }

            return null;
        }

        public static bool EnableValidation { get; }
        public static bool EnableGPUBasedValidation { get; }

        /// <summary>
        /// Gets the <see cref="GraphicsDeviceCapabilities" /> for the instance.
        /// </summary>
        public abstract GraphicsDeviceCapabilities Capabilities { get; }

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
