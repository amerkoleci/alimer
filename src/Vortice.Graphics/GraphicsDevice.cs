// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Toolkit.Diagnostics;

namespace Vortice.Graphics
{
    public abstract class GraphicsDevice : IDisposable
    {
        protected GraphicsDevice(GraphicsBackend backendType, PhysicalDevice physicalDevice)
        {
            Guard.IsNotNull(physicalDevice, nameof(physicalDevice));

            BackendType = backendType;
            PhysicalDevice = physicalDevice;
        }

        /// <summary>
        /// Get the device backend type.
        /// </summary>
        public GraphicsBackend BackendType { get; }

        /// <summary>
        /// Get the <see cref="Graphics.PhysicalDevice"/> that was used to create this device.
        /// </summary>
        public PhysicalDevice PhysicalDevice { get; }

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

        public SwapChain CreateSwapChain(in SwapChainSurface surface, in SwapChainDescriptor descriptor)
        {
            Guard.IsNotNull(surface, nameof(surface));

            return CreateSwapChainCore(surface, descriptor);
        }

        public Texture CreateTexture(in TextureDescriptor descriptor)
        {
            Guard.IsGreaterThanOrEqualTo(descriptor.Width, 1, nameof(TextureDescriptor.Width));
            Guard.IsGreaterThanOrEqualTo(descriptor.Height, 1, nameof(TextureDescriptor.Height));
            Guard.IsGreaterThanOrEqualTo(descriptor.DepthOrArraySize, 1, nameof(TextureDescriptor.DepthOrArraySize));

            return CreateTextureCore(descriptor);
        }

        protected abstract SwapChain CreateSwapChainCore(in SwapChainSurface surface, in SwapChainDescriptor descriptor);

        protected abstract Texture CreateTextureCore(in TextureDescriptor descriptor);
    }
}
