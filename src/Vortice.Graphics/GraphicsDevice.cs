// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;

namespace Vortice.Graphics
{
    public abstract class GraphicsDevice : IDisposable
    {
        public const string EnableValidationSwitchName = "Vortice.Graphics.EnableValidation";
        public const string EnableGPUBasedValidationSwitchName = "Vortice.Graphics.EnableGPUBasedValidation";

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

        /// <summary>
        /// Gets the best supported <see cref="BackendType"/> on the current platform.
        /// </summary>
        /// <returns>
        /// The best supported <see cref="BackendType"/>.
        /// </returns>
        public static BackendType GetDefaultPlatformBackend()
        {
            if (OperatingSystem.IsWindows())
            {
                if (IsBackendSupported(BackendType.Direct3D12))
                {
                    return BackendType.Direct3D12;
                }

                if (IsBackendSupported(BackendType.Vulkan))
                {
                    return BackendType.Vulkan;
                }

                return BackendType.Null;
            }
            else if (OperatingSystem.IsMacOS())
            {
            //    return BackendType.Metal;
            }

            if (IsBackendSupported(BackendType.Vulkan))
            {
                return BackendType.Vulkan;
            }

            return BackendType.Null;
        }

        /// <summary>
        /// Checks whether the given backend is supported on current operating system.
        /// </summary>
        /// <param name="backend">The <see cref="BackendType"/> to check.</param>
        /// <returns><c>true</c> if supported, <c>false</c> otherwise.</returns>
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

#if !EXCLUDE_VULKAN_BACKEND
                case BackendType.Vulkan:
                    return Vulkan.VulkanGraphicsDevice.IsSupported();
#endif

#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    return D3D12.D3D12GraphicsDevice.IsSupported();
#endif

                default:
                    return false;
            }
        }

        public static GraphicsDevice? CreateSystemDefault(BackendType preferredBackendType = BackendType.Count)
        {
            if (preferredBackendType == BackendType.Count)
            {
                preferredBackendType = GetDefaultPlatformBackend();
            }

            switch (preferredBackendType)
            {
#if !EXCLUDE_VULKAN_BACKEND
                case BackendType.Vulkan:
                    return new Vulkan.VulkanGraphicsDevice(PowerPreference.HighPerformance);
#endif

#if !EXCLUDE_D3D12_BACKEND
                case BackendType.Direct3D12:
                    return new D3D12.D3D12GraphicsDevice(PowerPreference.HighPerformance);
#endif

                default:
                    return new Null.NullGraphicsDevice();
            }
        }


        public SwapChain CreateSwapChain(IntPtr windowHandle, in SwapChainDescriptor descriptor)
        {
            return CreateSwapChainCore(windowHandle, descriptor);
        }

        protected abstract SwapChain CreateSwapChainCore(IntPtr windowHandle, in SwapChainDescriptor descriptor);
    }
}
