// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Runtime.InteropServices;

namespace Vortice.Graphics
{
    public abstract class GraphicsDevice : IDisposable
    {
        protected GraphicsDevice()
        {
        }

        public static bool EnableValidationLayers { get; set; }
        public static bool EnableGpuBasedValidation { get; set; }

        /// <summary>
        /// Get the device capabilities.
        /// </summary>
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

        public static GraphicsDevice? Create()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                if (IsSupported(GraphicsBackend.Direct3D12))
                {
#if !EXCLUDE_D3D12_BACKEND
                    return D3D12.D3D12DeviceHelper.DefaultDevice.Value;
#endif // !EXCLUDE_D3D12_BACKEND
                }
            }

            return null;
        }

        public static bool IsSupported(GraphicsBackend backend)
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
                    return D3D12.D3D12DeviceHelper.IsSupported.Value;
#endif // !EXCLUDE_D3D12_BACKEND
                default:
                    return false;
            }
        }


        public static GraphicsBackend GetDefaultPlatformBackend()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
#if !EXCLUDE_D3D12_BACKEND
                if (D3D12.D3D12DeviceHelper.IsSupported.Value)
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

        public Texture CreateTexture(in TextureDescriptor descriptor)
        {
            return CreateTextureCore(descriptor);
        }

        protected abstract Texture CreateTextureCore(in TextureDescriptor descriptor);
    }
}
