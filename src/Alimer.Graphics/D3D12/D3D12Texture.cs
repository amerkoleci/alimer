// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.D3D.D3DUtils;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_DSV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_HEAP_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_RTV_DIMENSION;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.Windows.Windows;
using DescriptorIndex = System.UInt32;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Texture : Texture, ID3D12GpuResource
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12Resource> _handle;
    private readonly ComPtr<D3D12MA_Allocation> _allocation;
    private HANDLE _sharedHandle = HANDLE.NULL;
    private readonly Dictionary<int, DescriptorIndex> _RTVs = new();
    private readonly Dictionary<int, DescriptorIndex> _DSVs = new();

    public D3D12Texture(D3D12GraphicsDevice device, in TextureDescription description, void* initialData)
        : base(description)
    {
        _device = device;
        DxgiFormat = (DXGI_FORMAT)description.Format.ToDxgiFormat();
        bool isDepthStencil = description.Format.IsDepthStencilFormat();

        // If ShaderRead or ShaderWrite and depth format, set to typeless.
        if (isDepthStencil && (description.Usage & TextureUsage.ShaderReadWrite) != 0)
        {
            DxgiFormat = description.Format.GetTypelessFormatFromDepthFormat();
        }

        D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;

        if ((description.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                if ((description.Usage & TextureUsage.ShaderRead) == 0)
                {
                    resourceFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                }
            }
            else
            {
                resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
        }

        if ((description.Usage & TextureUsage.ShaderWrite) != 0)
        {
            resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
        bool isShared = false;
        if ((description.Usage & TextureUsage.Shared) != 0)
        {
            heapFlags |= D3D12_HEAP_FLAG_SHARED;
            isShared = true;

            // TODO: What about D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER?
        }

        D3D12_RESOURCE_DESC resourceDesc = D3D12_RESOURCE_DESC.Tex2D(
            DxgiFormat,
            description.Width,
            description.Height,
            (ushort)description.DepthOrArrayLayers,
            (ushort)description.MipLevelCount,
            description.SampleCount.ToSampleCount(), 0,
            resourceFlags);
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        if (initialData == null)
        {
            if ((description.Usage & TextureUsage.RenderTarget) != 0)
            {
                if (isDepthStencil)
                {
                    initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                }
                else
                {
                    initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }
            else if ((description.Usage & TextureUsage.ShaderWrite) != 0)
            {
                initialState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            else if ((description.Usage & TextureUsage.ShaderRead) != 0)
            {
                initialState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            }
        }

        D3D12MA_ALLOCATION_DESC allocationDesc = new();
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        HRESULT hr = device.MemoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            initialState,
            null,
            _allocation.GetAddressOf(),
            __uuidof<ID3D12Resource>(),
            _handle.GetVoidAddressOf()
            );

        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Texture.");
            return;
        }

        if (isShared)
        {
            HANDLE sharedHandle = default;
            if (device.Handle->CreateSharedHandle((ID3D12DeviceChild*)_handle.Get(), null, GENERIC_ALL, null, &sharedHandle).FAILED)
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

    public D3D12Texture(D3D12GraphicsDevice device, ID3D12Resource* existingTexture, in TextureDescription descriptor)
        : base(descriptor)
    {
        _device = device;
        DxgiFormat = (DXGI_FORMAT)descriptor.Format.ToDxgiFormat();
        _handle = existingTexture;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public DXGI_FORMAT DxgiFormat { get; }
    public ID3D12Resource* Handle => _handle;
    public ResourceStates State { get; set; }
    public ResourceStates TransitioningState { get; set; } = (ResourceStates)uint.MaxValue;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Texture" /> class.
    /// </summary>
    ~D3D12Texture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        foreach (DescriptorIndex index in _RTVs.Values)
        {
            _device.RenderTargetViewHeap.ReleaseDescriptors(index);
        }
        _RTVs.Clear();

        foreach (DescriptorIndex index in _DSVs.Values)
        {
            _device.DepthStencilViewHeap.ReleaseDescriptors(index);
        }
        _DSVs.Clear();

        if (_sharedHandle.Value != null)
        {
            _ = CloseHandle(_sharedHandle);
        }

        _allocation.Dispose();
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

    public D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int mipSlice, int arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
    {
        int hash = HashCode.Combine(mipSlice, arraySlice, format);

        if (!_RTVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_RESOURCE_DESC desc = _handle.Get()->GetDesc();

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = default;
            viewDesc.Format = format;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            descriptorIndex = _device.RenderTargetViewHeap.AllocateDescriptors(1u);
            _RTVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _device.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
            _device.Handle->CreateRenderTargetView(_handle.Get(), &viewDesc, cpuHandle);
        }

        return _device.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(int mipSlice, int arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
    {
        int hash = HashCode.Combine(mipSlice, arraySlice, format);

        if (!_DSVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_RESOURCE_DESC desc = _handle.Get()->GetDesc();

            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = default;
            viewDesc.Format = format;
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            descriptorIndex = _device.DepthStencilViewHeap.AllocateDescriptors(1u);
            _DSVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _device.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
            _device.Handle->CreateDepthStencilView(_handle.Get(), &viewDesc, cpuHandle);
        }

        return _device.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
    }
}
