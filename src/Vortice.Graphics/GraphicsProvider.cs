// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Runtime.InteropServices;

namespace Vortice.Graphics
{
    public abstract class GraphicsProvider : IDisposable
    {
        /// <inheritdoc />
        public void Dispose()
        {
            Dispose(isDisposing: true);
            GC.SuppressFinalize(this);
        }

        /// <inheritdoc cref="Dispose()" />
        /// <param name="isDisposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
        protected abstract void Dispose(bool isDisposing);

        public static GraphicsProvider? Create(GraphicsBackend backend = GraphicsBackend.Default, bool validation = false)
        {
            if (backend == GraphicsBackend.Default)
            {
                backend = GetDefaultPlatformBackend();
            }

            if (IsSupported(backend))
            {
                throw new GraphicsException($"Backend {backend} is not supported");
            }

            switch (backend)
            {
#if !EXCLUDE_VULKAN_BACKEND
                case GraphicsBackend.Vulkan:
                    return new Vulkan.VulkanGraphicsProvider(validation);
#endif // !EXCLUDE_VULKAN_BACKEND

#if !EXCLUDE_D3D12_BACKEND
                case GraphicsBackend.Direct3D12:
                    return new D3D12.D3D12GraphicsProvider(validation);
#endif // !EXCLUDE_D3D12_BACKEND
                default:
                    return null;
            }
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
                    return Vulkan.VulkanGraphicsProvider.IsSupported();
#endif // !EXCLUDE_VULKAN_BACKEND

#if !EXCLUDE_D3D12_BACKEND
                case GraphicsBackend.Direct3D12:
                    return D3D12.D3D12GraphicsProvider.IsSupported();
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
                if (D3D12.D3D12GraphicsProvider.IsSupported())
                {
                    return GraphicsBackend.Direct3D12;
                }
#endif // !EXCLUDE_D3D12_BACKEND
            }

#if !EXCLUDE_VULKAN_BACKEND
            if (Vulkan.VulkanGraphicsProvider.IsSupported())
            {
                return GraphicsBackend.Vulkan;
            }
#endif // !EXCLUDE_VULKAN_BACKEND

            return GraphicsBackend.Vulkan;
        }
    }
}
