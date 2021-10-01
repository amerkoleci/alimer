// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    public abstract class PhysicalDevice
    {
        protected PhysicalDevice()
        {
        }

        public abstract VendorId VendorId { get; }
        public abstract uint AdapterId { get; }
        public abstract GPUAdapterType AdapterType { get; }
        public abstract string AdapterName { get; }

        public abstract GraphicsDevice CreateDevice(string? name = null);
    }
}
