// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;

namespace Vortice.Graphics
{
    /// <summary>
    /// Factory for creating surface and providing physical devices.
    /// </summary>
    public abstract class GraphicsDeviceFactory : IDisposable
    {
        protected GraphicsDeviceFactory(ValidationMode validationMode)
        {
            ValidationMode = validationMode;
        }

        public ValidationMode ValidationMode { get; }

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
        /// Create new instance of <see cref="GraphicsDeviceFactory"/> class.
        /// </summary>
        /// <param name="validationMode">The type of validation to use.</param>
        /// <returns>Instance of <see cref="GraphicsDeviceFactory"/> class.</returns>
        public static GraphicsDeviceFactory Create(ValidationMode validationMode = ValidationMode.Disabled)
        {
            if (OperatingSystem.IsWindows())
            {
                if (IsBackendSupported(GraphicsBackend.Direct3D12))
                {
#if !EXCLUDE_D3D12_BACKEND
                    return new D3D12.D3D12GraphicsDeviceFactory(validationMode);
#endif // !EXCLUDE_D3D12_BACKEND
                }
            }

            if (IsBackendSupported(GraphicsBackend.Vulkan))
            {
#if !EXCLUDE_VULKAN_BACKEND
                return Vulkan.VulkanDeviceHelper.DefaultDevice.Value;
#endif // !EXCLUDE_VULKAN_BACKEND
            }

            return null;
        }

        public static bool IsBackendSupported(GraphicsBackend backend)
        {
            if (backend == GraphicsBackend.Default)
            {
                backend = GetDefaultPlatformBackend();
            }

            switch (backend)
            {
#if !EXCLUDE_VULKAN_BACKEND
                case GraphicsBackend.Vulkan:
                    return Vulkan.VulkanDeviceHelper.IsSupported.Value;
#endif // !EXCLUDE_VULKAN_BACKEND

#if !EXCLUDE_D3D12_BACKEND
                case GraphicsBackend.Direct3D12:
                    return D3D12.D3D12GraphicsDeviceFactory.IsSupported.Value;
#endif // !EXCLUDE_D3D12_BACKEND

                default:
                    return false;
            }
        }

        public static GraphicsBackend GetDefaultPlatformBackend()
        {
            if (OperatingSystem.IsWindows())
            {
#if !EXCLUDE_D3D12_BACKEND
                if (D3D12.D3D12GraphicsDeviceFactory.IsSupported.Value)
                {
                    return GraphicsBackend.Direct3D12;
                }
#endif // !EXCLUDE_D3D12_BACKEND
            }

#if !EXCLUDE_VULKAN_BACKEND
            if (Vulkan.VulkanDeviceHelper.IsSupported.Value)
            {
                return GraphicsBackend.Vulkan;
            }
#endif // !EXCLUDE_VULKAN_BACKEND

            return GraphicsBackend.Vulkan;
        }

        public abstract GraphicsSurface CreateSurface(in SurfaceSource source);

        public abstract GraphicsAdapter? RequestAdapter(GPUPowerPreference powerPreference = GPUPowerPreference.HighPerformance);
    }
}
