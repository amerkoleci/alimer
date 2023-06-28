// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D11;
using Win32.Graphics.Dxgi.Common;
using D3D11Usage = Win32.Graphics.Direct3D11.Usage;
using static Alimer.Graphics.D3D.D3DUtils;
using static Win32.Graphics.Dxgi.Apis;
using Win32.Graphics.Dxgi;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11Texture : Texture
{
    private readonly ComPtr<ID3D11Resource> _handle;
    private Handle _sharedHandle = Win32.Handle.Null;

    public D3D11Texture(D3D11GraphicsDevice device, in TextureDescriptor descriptor, void* initialData)
        : base(device, descriptor)
    {
        DxgiFormat = (Format)descriptor.Format.ToDxgiFormat();
        bool isDepthStencil = descriptor.Format.IsDepthStencilFormat();

        D3D11Usage usage = D3D11Usage.Default;
        BindFlags bindFlags = 0;
        CpuAccessFlags cpuAccessFlags = 0u;
        ResourceMiscFlags miscFlags = ResourceMiscFlags.None;

        if ((descriptor.Usage & TextureUsage.ShaderRead) != 0)
        {
            bindFlags |= BindFlags.ShaderResource;
        }

        if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
        {
            bindFlags |= BindFlags.UnorderedAccess;
        }

        if ((descriptor.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                bindFlags |= BindFlags.DepthStencil;
            }
            else
            {
                bindFlags |= BindFlags.RenderTarget;
            }
        }

        // If ShaderRead or ShaderWrite and depth format, set to typeless.
        if (isDepthStencil && (descriptor.Usage & TextureUsage.ShaderReadWrite) != 0)
        {
            DxgiFormat = (Format)descriptor.Format.GetTypelessFormatFromDepthFormat();
        }

        bool isShared = false;
        if ((descriptor.Usage & TextureUsage.Shared) != 0)
        {
            miscFlags |= ResourceMiscFlags.SharedKeyedMutex | ResourceMiscFlags.SharedNtHandle;
            isShared = true;
        }

        HResult hr = HResult.Fail;
        switch (descriptor.Dimension)
        {
            case TextureDimension.Texture2D:
                Texture2DDescription desc2d = new();
                desc2d.Width = (uint)descriptor.Width;
                desc2d.Height = (uint)descriptor.Height;
                desc2d.MipLevels = (uint)descriptor.MipLevelCount;
                desc2d.ArraySize = (uint)descriptor.DepthOrArrayLayers;
                desc2d.Format = DxgiFormat;
                desc2d.SampleDesc = new(descriptor.SampleCount.ToSampleCount(), 0);
                desc2d.Usage = usage;
                desc2d.BindFlags = bindFlags;
                desc2d.CPUAccessFlags = cpuAccessFlags;

                if (descriptor.DepthOrArrayLayers >= 6 && descriptor.Width == descriptor.Height)
                    desc2d.MiscFlags = miscFlags | ResourceMiscFlags.TextureCube;
                else
                    desc2d.MiscFlags = miscFlags;

                hr = device.Handle->CreateTexture2D(&desc2d, null, (ID3D11Texture2D**)_handle.GetAddressOf());

                break;

            default:
                throw new NotImplementedException();
        }

        if (hr.Failure)
        {
            //return nullptr;
        }

        if (isShared)
        {
            using ComPtr<IDXGIResource1> dxgiResource1 = default;
            if (_handle.CopyTo(dxgiResource1.GetAddressOf()).Success)
            {
                Handle sharedHandle = Win32.Handle.Null;
                if (dxgiResource1.Get()->CreateSharedHandle(null, GENERIC_ALL, null, &sharedHandle).Failure)
                {
                    return;
                }

                _sharedHandle = sharedHandle;
            }
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public D3D11Texture(GraphicsDevice device, ID3D11Resource* existingTexture, in TextureDescriptor descriptor)
        : base(device, descriptor)
    {
        DxgiFormat = (Format)descriptor.Format.ToDxgiFormat();
        _handle = existingTexture;
    }

    public Format DxgiFormat { get; }
    public ID3D11Resource* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D11Texture" /> class.
    /// </summary>
    ~D3D11Texture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (_sharedHandle.Value != null)
        {
            _ = CloseHandle(_sharedHandle);
        }

        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _handle.Get()->SetDebugName(newLabel);
    }
}
