﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    public unsafe class TextureD3D12 : Texture
    {
        public TextureD3D12(GraphicsDeviceD3D12 device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
            ResourceDescription resourceDesc = new()
            {
                Dimension = descriptor.Dimension.ToD3D12(),
                Alignment = 0u,
                Width = (ulong)descriptor.Width,
                Height = descriptor.Height,
                DepthOrArraySize = (ushort)descriptor.DepthOrArraySize,
                MipLevels = (ushort)descriptor.MipLevels,
                Format = descriptor.Format.ToDXGIFormat(),
                SampleDescription = new SampleDescription(descriptor.SampleCount.ToD3D12(), 0),
                Layout = TextureLayout.Unknown,
                Flags = ResourceFlags.None
            };

            if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
            {
                resourceDesc.Flags |= ResourceFlags.AllowUnorderedAccess;
            }

            ResourceStates state = ResourceStates.Common;
            ClearValue clearValue = default;
            ClearValue? optimizedClearValue = null;
            if ((descriptor.Usage & TextureUsage.RenderTarget) == TextureUsage.RenderTarget)
            {
                clearValue.Format = resourceDesc.Format;

                if (descriptor.Format.IsDepthStencilFormat())
                {
                    state = ResourceStates.DepthWrite;
                    resourceDesc.Flags |= ResourceFlags.AllowDepthStencil;
                    if ((descriptor.Usage & TextureUsage.ShaderRead) == TextureUsage.None)
                    {
                        resourceDesc.Flags |= ResourceFlags.DenyShaderResource;
                    }

                    clearValue.DepthStencil.Depth = 1.0f;
                }
                else
                {
                    state = ResourceStates.RenderTarget;
                    resourceDesc.Flags |= ResourceFlags.AllowRenderTarget;
                }

                optimizedClearValue = clearValue;
            }

            // If depth and either sampled or storage, set to typeless
            if (descriptor.Format.IsDepthFormat())
            {
                if ((descriptor.Usage & TextureUsage.ShaderRead) != 0
                    || (descriptor.Usage & TextureUsage.ShaderWrite) != 0)
                {
                    resourceDesc.Format = descriptor.Format.GetTypelessFormatFromDepthFormat();
                    optimizedClearValue = null;
                }
            }

            Handle = device.NativeDevice.CreateCommittedResource<ID3D12Resource>(
                HeapProperties.DefaultHeapProperties,
                HeapFlags.None,
                resourceDesc,
                state,
                optimizedClearValue);
        }

        public ID3D12Resource Handle { get; }

        //public D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Handle.Dispose();
                //allocation.Dispose();
            }
        }
    }
}
