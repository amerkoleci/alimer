// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.D3D12_HEAP_TYPE;
using static TerraFX.Interop.D3D12_HEAP_FLAGS;
using static TerraFX.Interop.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.D3D12_TEXTURE_LAYOUT;
using static TerraFX.Interop.D3D12_RESOURCE_STATES;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12Texture : Texture
    {
        private readonly ComPtr<ID3D12Resource> _handle;

        public D3D12Texture(D3D12GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
            D3D12_RESOURCE_DESC resourceDesc = new()
            {
                Dimension = descriptor.Dimension.ToD3D12(),
                Alignment = 0u,
                Width = (ulong)descriptor.Width,
                Height = (uint)descriptor.Height,
                DepthOrArraySize = (ushort)descriptor.DepthOrArraySize,
                MipLevels = (ushort)descriptor.MipLevels,
                Format = descriptor.Format.ToDXGIFormat(),
                SampleDesc = new(descriptor.SampleCount.ToD3D12(), 0),
                Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                Flags = D3D12_RESOURCE_FLAG_NONE
            };

            if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
            D3D12_CLEAR_VALUE clearValue = default;
            D3D12_CLEAR_VALUE* optimizedClearValue = null;
            if ((descriptor.Usage & TextureUsage.RenderTarget) == TextureUsage.RenderTarget)
            {
                clearValue.Format = resourceDesc.Format;

                if (descriptor.Format.IsDepthStencilFormat())
                {
                    state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                    if ((descriptor.Usage & TextureUsage.ShaderRead) == TextureUsage.None)
                    {
                        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                    }

                    clearValue.DepthStencil.Depth = 1.0f;
                }
                else
                {
                    state = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }

                optimizedClearValue = &clearValue;
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

            D3D12_HEAP_PROPERTIES heapProps = default;
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

            device.NativeDevice->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                state,
                optimizedClearValue,
                __uuidof<ID3D12Resource>(),
                _handle.GetVoidAddressOf()).Assert(); 
        }

        public ID3D12Resource* Handle => _handle;

        //public D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _handle.Dispose();
                //allocation.Dispose();
            }
        }
    }
}
