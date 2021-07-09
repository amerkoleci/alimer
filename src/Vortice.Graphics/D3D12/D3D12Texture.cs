// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.D3D12_HEAP_TYPE;
using static TerraFX.Interop.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.D3D12MA_ALLOCATION_FLAGS;
using static TerraFX.Interop.D3D12_TEXTURE_LAYOUT;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12Texture : Texture
    {
        private ComPtr<ID3D12Resource> resource;
        private UniquePtr<D3D12MA_Allocation> allocation;

        public D3D12Texture(D3D12GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
            D3D12MA_ALLOCATION_DESC allocationDesc = default;
            allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            allocationDesc.Flags = D3D12MA_ALLOCATION_FLAG_NONE;

            D3D12_RESOURCE_DESC resourceDesc = new D3D12_RESOURCE_DESC()
            {
                Dimension = descriptor.Dimension.ToD3D12(),
                Alignment = 0u,
                Width = (ulong)descriptor.Width,
                Height = (uint)descriptor.Height,
                DepthOrArraySize = (ushort)descriptor.DepthOrArraySize,
                MipLevels = (ushort)descriptor.MipLevels,
                Format = descriptor.Format.ToDXGIFormat(),
                SampleDesc = new DXGI_SAMPLE_DESC(descriptor.SampleCount.ToD3D12(), 0),
                Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                Flags = D3D12_RESOURCE_FLAG_NONE
            };

            //if ((descriptor.Usage & TextureUsage.Storage) == TextureUsage.Storage)
            //{
            //    description.Flags |= ResourceFlags.AllowUnorderedAccess;
            //}

#if TODO
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
#endif

            HRESULT hr = device.Allocator->CreateResource(
                &allocationDesc,
                &resourceDesc,
                State,
                null,
                allocation.GetAddressOf(),
                null,
                null);

            hr.Assert();
        }

        public ID3D12Resource* Resource => resource;

        public D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                resource.Dispose();
                allocation.Dispose();
            }
        }
    }
}
