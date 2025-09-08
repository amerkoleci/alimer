// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUTexture : Texture
{
    private readonly WebGPUGraphicsDevice _device;
    private readonly Dictionary<int, WGPUTextureView> _views = [];

    public WebGPUTexture(WebGPUGraphicsDevice device, in TextureDescriptor description, TextureData* initialData)
        : base(description)
    {
        _device = device;
        WebGPUFormat = description.Format.ToWebGPU();
        WGPUTextureUsage usage = WGPUTextureUsage.None;
        WGPUTextureDimension dimension = description.Dimension.ToWebGPU();
        WGPUExtent3D size = new(description.Width, description.Height, description.DepthOrArrayLayers);

        if ((description.Usage & TextureUsage.Transient) != 0)
        {
            //usage |= WGPUTextureUsage.TransientAttachment;
        }
        else
        {
            usage |= WGPUTextureUsage.CopySrc;
            usage |= WGPUTextureUsage.CopyDst;
        }

        if ((description.Usage & TextureUsage.ShaderRead) != 0)
        {
            usage |= WGPUTextureUsage.TextureBinding;
        }

        if ((description.Usage & TextureUsage.ShaderWrite) != 0)
        {
            usage |= WGPUTextureUsage.StorageBinding;
        }

        if ((description.Usage & TextureUsage.RenderTarget) != 0)
        {
            usage |= WGPUTextureUsage.RenderAttachment;
        }

        //if ((description.Usage & TextureUsage.ShadingRate) != 0)
        //{
        //    usage |= WGPUTextureUsage.FragmentShadingRateAttachmentKHR;
        //}
        //
        //if (!isDepthStencil && (description.Usage & (TextureUsage.ShaderRead | TextureUsage.RenderTarget)) != 0)
        //{
        //    usage |= WGPUTextureUsage.InputAttachment;
        //}

        WGPUTextureDescriptor descriptor = new()
        {
            usage = usage,
            dimension = dimension,
            size = size,
            format = WebGPUFormat,
            mipLevelCount = MipLevelCount,
            sampleCount = SampleCount.ToWebGPU(),
            viewFormatCount = 0,
            viewFormats = null
        };

        Handle = wgpuDeviceCreateTexture(device.Handle, &descriptor);

        if (Handle.IsNull)
        {
            Log.Error("WebGPU: Failed to create texture.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    public WebGPUTexture(WebGPUGraphicsDevice device, WGPUTexture existingTexture, in TextureDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        Handle = existingTexture;
        WebGPUFormat = descriptor.Format.ToWebGPU();

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public WGPUTexture Handle { get; }
    public WGPUTextureFormat WebGPUFormat { get; }
    public ResourceStates CurrentState { get; set; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUTexture" /> class.
    /// </summary>
    ~WebGPUTexture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        foreach (var view in _views.Values)
        {
            wgpuTextureViewRelease(view);
        }
        _views.Clear();

        wgpuTextureRelease(Handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuTextureSetLabel(Handle, newLabel);
    }

    public WGPUTextureView GetView(int baseMipLevel, int baseArrayLayer = 0, uint mipLevelCount = (uint)WGPU_MIP_LEVEL_COUNT_UNDEFINED, uint arrayLayerCount = (uint)WGPU_ARRAY_LAYER_COUNT_UNDEFINED)
    {
        int hash = HashCode.Combine(baseMipLevel, baseArrayLayer, mipLevelCount, arrayLayerCount);

        if (!_views.TryGetValue(hash, out WGPUTextureView view))
        {
            WGPUTextureAspect aspect = WebGPUFormat.GetWGPUTextureAspect();
            WGPUTextureViewDescriptor descriptor = new()
            {
                format = WebGPUFormat,
                dimension = WGPUTextureViewDimension._2D,
                baseMipLevel = (uint)baseMipLevel,
                mipLevelCount = mipLevelCount,
                baseArrayLayer = (uint)baseArrayLayer,
                arrayLayerCount = arrayLayerCount,
                aspect = aspect,
            };

            view = wgpuTextureCreateView(Handle, &descriptor);
            if (view.IsNull)
            {
                Log.Error($"WebGPU: Failed to create TextureView");
                return WGPUTextureView.Null;
            }

            _views.Add(hash, view);
        }

        return view;
    }
}
