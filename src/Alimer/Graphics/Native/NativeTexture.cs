// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Native.AlimerGPUApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeTexture : Texture
{
    private readonly NativeGraphicsDevice _device;

    public NativeTexture(NativeGraphicsDevice device, in TextureDescriptor descriptor, TextureData* initialData)
        : base(in descriptor)
    {
        _device = device;
        GPUTextureDesc nativeDesc = new()
        {
            dimension = ToNative(descriptor.Dimension),
            format = descriptor.Format,
            width = descriptor.Width,
            height = descriptor.Height,
            depthOrArrayLayers = descriptor.DepthOrArrayLayers,
            mipLevelCount = descriptor.MipLevelCount,
            sampleCount = (uint)descriptor.SampleCount,
            usage = descriptor.Usage
        };

        GPUTextureData nativeTextureData = default;
        if (initialData != null)
        {
            nativeTextureData.pData = initialData->DataPointer.ToPointer();
            nativeTextureData.rowPitch = initialData->RowPitch;
            nativeTextureData.slicePitch = initialData->SlicePitch;
        }
        Handle = agpuDeviceCreateTexture(device.Handle, &nativeDesc, initialData is not null ? &nativeTextureData : null);
    }

    private static GPUTextureDimension ToNative(TextureDimension dimension)
    {
        return dimension switch
        {
            TextureDimension.Texture1D => GPUTextureDimension.Texture1D,
            TextureDimension.Texture2D => GPUTextureDimension.Texture2D,
            TextureDimension.Texture3D => GPUTextureDimension.Texture3D,
            TextureDimension.TextureCube => GPUTextureDimension.TextureCube,
            _ => GPUTextureDimension.Undefined
        };
    }

    public override GraphicsDevice Device => _device;
    public GPUTexture Handle { get; }

    protected internal override void Destroy() => throw new NotImplementedException();
    protected override TextureView CreateView(in TextureViewDescriptor descriptor) => throw new NotImplementedException();
}
