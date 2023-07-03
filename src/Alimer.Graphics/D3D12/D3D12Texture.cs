// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D12;
using Win32.Graphics.Dxgi.Common;
using static Win32.Apis;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Alimer.Graphics.D3D.D3DUtils;
using D3DResourceStates = Win32.Graphics.Direct3D12.ResourceStates;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Texture : Texture
{
    private readonly ComPtr<ID3D12Resource> _handle;
    private Handle _sharedHandle = Win32.Handle.Null;
    private readonly Dictionary<int, CpuDescriptorHandle> _RTVs = new();
    private readonly Dictionary<int, CpuDescriptorHandle> _DSVs = new();

    public D3D12Texture(D3D12GraphicsDevice device, in TextureDescription description, void* initialData)
        : base(device, description)
    {
        DxgiFormat = (Format)description.Format.ToDxgiFormat();
        bool isDepthStencil = description.Format.IsDepthStencilFormat();

        // If ShaderRead or ShaderWrite and depth format, set to typeless.
        if (isDepthStencil && (description.Usage & TextureUsage.ShaderReadWrite) != 0)
        {
            DxgiFormat = (Format)description.Format.GetTypelessFormatFromDepthFormat();
        }

        HeapProperties heapProps = DefaultHeapProps;
        ResourceFlags resourceFlags = ResourceFlags.None;

        if ((description.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                resourceFlags |= ResourceFlags.AllowDepthStencil;
                if ((description.Usage & TextureUsage.ShaderRead) == 0)
                {
                    resourceFlags |= ResourceFlags.DenyShaderResource;
                }
            }
            else
            {
                resourceFlags |= ResourceFlags.AllowRenderTarget;
            }
        }

        if ((description.Usage & TextureUsage.ShaderWrite) != 0)
        {
            resourceFlags |= ResourceFlags.AllowUnorderedAccess;
        }

        HeapFlags heapFlags = HeapFlags.None;
        bool isShared = false;
        if ((description.Usage & TextureUsage.Shared) != 0)
        {
            heapFlags |= HeapFlags.Shared;
            isShared = true;

            // TODO: What about D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER?
        }

        ResourceDescription resourceDesc = ResourceDescription.Tex2D(
            DxgiFormat,
            description.Width,
            description.Height,
            (ushort)description.DepthOrArrayLayers,
            (ushort)description.MipLevelCount,
            description.SampleCount.ToSampleCount(), 0,
            resourceFlags);
        D3DResourceStates initialState = D3DResourceStates.Common;

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
            Log.Error("D3D12: Failed to create Texture.");
            return;
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

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    public D3D12Texture(GraphicsDevice device, ID3D12Resource* existingTexture, in TextureDescription descriptor)
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
        foreach (CpuDescriptorHandle view in _RTVs.Values)
        {
            ((D3D12GraphicsDevice)Device).FreeDescriptor(DescriptorHeapType.Rtv, in view);
        }
        _RTVs.Clear();

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

    public CpuDescriptorHandle GetRTV(int mipSlice, int arraySlice, Format format = Win32.Graphics.Dxgi.Common.Format.Unknown)
    {
        int hash = HashCode.Combine(mipSlice, arraySlice, format);

        if (!_RTVs.TryGetValue(hash, out CpuDescriptorHandle rtvHandle))
        {
            ResourceDescription desc = _handle.Get()->GetDesc();

            RenderTargetViewDescription viewDesc = default;
            viewDesc.Format = format;
            viewDesc.ViewDimension = RtvDimension.Texture2D;

            rtvHandle = ((D3D12GraphicsDevice)Device).AllocateDescriptor(DescriptorHeapType.Rtv);
            ((D3D12GraphicsDevice)Device).Handle->CreateRenderTargetView(_handle.Get(), &viewDesc, rtvHandle);

            _RTVs.Add(hash, rtvHandle);
        }

        return rtvHandle;
    }
}
