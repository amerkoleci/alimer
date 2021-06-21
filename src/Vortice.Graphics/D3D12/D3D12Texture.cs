// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using Vortice.Direct3D12;

namespace Vortice.Graphics.D3D12
{
    internal class D3D12Texture : Texture
    {
        public D3D12Texture(D3D12GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
            ResourceDescription description = new ResourceDescription
            {
                Dimension = descriptor.Dimension.ToD3D12(),
                Alignment = 0u,
                Width = (ulong)descriptor.Width,
                Height = descriptor.Height,
                DepthOrArraySize = (ushort)descriptor.DepthOrArraySize,
                MipLevels = (ushort)descriptor.MipLevels,
                Format = descriptor.Format.ToDXGIFormat(),
                SampleDescription = new DXGI.SampleDescription(descriptor.SampleCount.ToD3D12(), 0),
                Layout = TextureLayout.Unknown,
                Flags = ResourceFlags.None
            };

            if ((descriptor.Usage & TextureUsage.Storage) == TextureUsage.Storage)
            {
                description.Flags |= ResourceFlags.AllowUnorderedAccess;
            }

            ResourceStates state = ResourceStates.Common;
            ClearValue clearValue = default;
            ClearValue? optimizedClearValue = null;
            if ((descriptor.Usage & TextureUsage.RenderTarget) == TextureUsage.RenderTarget)
            {
                clearValue.Format = description.Format;

                if (descriptor.Format.IsDepthStencilFormat())
                {
                    state = ResourceStates.DepthWrite;
                    description.Flags |= ResourceFlags.AllowDepthStencil;
                    if ((descriptor.Usage & TextureUsage.Sampled) == TextureUsage.None)
                    {
                        description.Flags |= ResourceFlags.DenyShaderResource;
                    }

                    clearValue.DepthStencil.Depth = 1.0f;
                }
                else
                {
                    state = ResourceStates.RenderTarget;
                    description.Flags |= ResourceFlags.AllowRenderTarget;
                }

                optimizedClearValue = clearValue;
            }

            // If depth and either sampled or storage, set to typeless
            if (descriptor.Format.IsDepthFormat())
            {
                if ((descriptor.Usage & TextureUsage.Sampled) == TextureUsage.Sampled
                    || (descriptor.Usage & TextureUsage.Storage) == TextureUsage.Storage)
                {
                    description.Format = descriptor.Format.GetTypelessFormatFromDepthFormat();
                    optimizedClearValue = null;
                }
            }

            Handle = device.NativeDevice.CreateCommittedResource<ID3D12Resource>(
                HeapProperties.DefaultHeapProperties,
                HeapFlags.None,
                description,
                state,
                optimizedClearValue);
        }

        public ID3D12Resource Handle { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
            }
        }
    }
}
