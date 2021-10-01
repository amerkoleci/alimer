// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D11;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D11
{
    internal class D3D11Texture : Texture
    {
        public D3D11Texture(D3D11GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device, descriptor)
        {
            Texture2DDescription resourceDesc = new()
            {
                Width = descriptor.Width,
                Height = descriptor.Height,
                MipLevels = descriptor.MipLevels,
                ArraySize = descriptor.DepthOrArraySize,
                Format = descriptor.Format.ToDXGIFormat(),
                SampleDescription = new SampleDescription(descriptor.SampleCount.ToD3D12(), 0),
                Usage = ResourceUsage.Default,
                BindFlags = BindFlags.ShaderResource,
                CpuAccessFlags = CpuAccessFlags.None,
                OptionFlags = ResourceOptionFlags.None
            };

            Handle = device.NativeDevice.CreateTexture2D(resourceDesc);
        }

        public ID3D11Resource Handle { get; }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Handle.Dispose();
            }
        }
    }
}
