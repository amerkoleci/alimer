// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D12;
using Win32.Graphics.Dxgi.Common;
using static Win32.Apis;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Alimer.Graphics.D3D.D3DUtils;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Texture : Texture
{
    private readonly ComPtr<ID3D12Resource> _handle;
    private Handle _sharedHandle = Win32.Handle.Null;

    public D3D12Texture(D3D12GraphicsDevice device, in TextureDescriptor descriptor, void* initialData)
        : base(device, descriptor)
    {
        DxgiFormat = (Format)descriptor.Format.ToDxgiFormat();
        bool isDepthStencil = descriptor.Format.IsDepthStencilFormat();

        // If ShaderRead or ShaderWrite and depth format, set to typeless.
        if (isDepthStencil && (descriptor.Usage & TextureUsage.ShaderReadWrite) != 0)
        {
            DxgiFormat = (Format)descriptor.Format.GetTypelessFormatFromDepthFormat();
        }

        HeapProperties heapProps = DefaultHeapProps;
        ResourceFlags resourceFlags = ResourceFlags.None;

        if ((descriptor.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                resourceFlags |= ResourceFlags.AllowDepthStencil;
                if ((descriptor.Usage & TextureUsage.ShaderRead) == 0)
                {
                    resourceFlags |= ResourceFlags.DenyShaderResource;
                }
            }
            else
            {
                resourceFlags |= ResourceFlags.AllowRenderTarget;
            }
        }

        if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
        {
            resourceFlags |= ResourceFlags.AllowUnorderedAccess;
        }

        HeapFlags heapFlags = HeapFlags.None;
        bool isShared = false;
        if ((descriptor.Usage & TextureUsage.Shared) != 0)
        {
            heapFlags |= HeapFlags.Shared;
            isShared = true;

            // TODO: What about D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER?
        }

        ResourceDescription resourceDesc = ResourceDescription.Tex2D(
            DxgiFormat,
            descriptor.Width,
            descriptor.Height,
            (ushort)descriptor.DepthOrArrayLayers,
            (ushort)descriptor.MipLevelCount,
            descriptor.SampleCount.ToSampleCount(), 0,
            resourceFlags);
        ResourceStates initialState = ResourceStates.Common;

        HResult hr = device.Handle->CreateCommittedResource(&heapProps,
            heapFlags,
            &resourceDesc,
            initialState,
            null,
            __uuidof<ID3D12Resource>(),
            _handle.GetVoidAddressOf()
            );

        if (hr.Failure)
        {
            //return nullptr;
        }

        if (isShared)
        {
            Handle sharedHandle = Win32.Handle.Null;
            if (device.Handle->CreateSharedHandle((ID3D12DeviceChild*)_handle.Get(), null, GENERIC_ALL, null, &sharedHandle).Failure)
            {
                return;
            }

            _sharedHandle = sharedHandle;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public D3D12Texture(GraphicsDevice device, ID3D12Resource* existingTexture, in TextureDescriptor descriptor)
        : base(device, descriptor)
    {
        DxgiFormat = (Format)descriptor.Format.ToDxgiFormat();
        _handle = existingTexture;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public Format DxgiFormat { get; }
    public ID3D12Resource* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Texture" /> class.
    /// </summary>
    ~D3D12Texture() => Dispose(disposing: false);

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
        fixed (char* pName = newLabel)
        {
            _ = _handle.Get()->SetName((ushort*)pName);
        }
    }
}
