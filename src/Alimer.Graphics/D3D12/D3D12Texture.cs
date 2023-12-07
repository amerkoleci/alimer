// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.D3D.D3DUtils;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_LAYOUT;
using static TerraFX.Interop.DirectX.D3D12_DSV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_HEAP_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_RTV_DIMENSION;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.Windows.Windows;
using static Alimer.Utilities.MemoryUtilities;
using DescriptorIndex = System.UInt32;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Texture : Texture, ID3D12GpuResource
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12Resource> _handle;
    private readonly ComPtr<D3D12MA_Allocation> _allocation;
    private HANDLE _sharedHandle = HANDLE.NULL;
    private readonly D3D12_PLACED_SUBRESOURCE_FOOTPRINT* _footprints;
    private readonly ulong* _rowSizesInBytes;
    private readonly uint* _numRows;
    private readonly void* pMappedData = default;

    private readonly Dictionary<int, DescriptorIndex> _RTVs = new();
    private readonly Dictionary<int, DescriptorIndex> _DSVs = new();

    public D3D12Texture(D3D12GraphicsDevice device, in TextureDescription description, TextureData* initialData)
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

        D3D12_RESOURCE_STATES initialState = description.InitialLayout.ToD3D12();
        if (initialData != null)
        {
            initialState = D3D12_RESOURCE_STATE_COMMON;
        }

        D3D12_RESOURCE_DESC resourceDesc = new();
        resourceDesc.Dimension = description.Dimension.ToD3D12();
        resourceDesc.Alignment = 0;
        resourceDesc.Width = description.Width;
        resourceDesc.Height = description.Height;
        resourceDesc.DepthOrArraySize = (ushort)description.DepthOrArrayLayers;
        resourceDesc.MipLevels = (ushort)description.MipLevelCount;
        resourceDesc.Format = DxgiFormat;
        resourceDesc.SampleDesc = new(description.SampleCount.ToSampleCount(), 0);
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = resourceFlags;

        ulong allocatedSize = 0;
        uint numSubResources = Math.Max(1u, resourceDesc.MipLevels) * resourceDesc.DepthOrArraySize;
        _footprints = AllocateArray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>(numSubResources);
        _rowSizesInBytes = AllocateArray<ulong>(numSubResources);
        _numRows = AllocateArray<uint>(numSubResources);
        device.Handle->GetCopyableFootprints(
            &resourceDesc,
            0,
            numSubResources,
            0,
            _footprints,
            _numRows,
            _rowSizesInBytes,
            &allocatedSize
        );
        AllocatedSize = allocatedSize;

        D3D12_CLEAR_VALUE clearValue = default;
        D3D12_CLEAR_VALUE* pClearValue = null;

        if ((description.Usage & TextureUsage.RenderTarget) != 0)
        {
            clearValue.Format = resourceDesc.Format;
            if (isDepthStencil)
            {
                clearValue.DepthStencil.Depth = 1.0f;
            }
            pClearValue = &clearValue;
        }

        // If shader read/write and depth format, set to typeless
        if (isDepthStencil && (description.Usage & TextureUsage.ShaderReadWrite) != 0)
        {
            pClearValue = null;
        }

        D3D12MA_ALLOCATION_DESC allocationDesc = new();
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        HRESULT hr = device.MemoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            initialState,
            pClearValue,
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

        // Issue data copy on request
        if (initialData != null)
        {
            D3D12_SUBRESOURCE_DATA* data = stackalloc D3D12_SUBRESOURCE_DATA[(int)numSubResources];
            for (uint i = 0; i < numSubResources; ++i)
            {
                ref TextureData subresourceData = ref initialData[i];

                data[i].pData = subresourceData.DataPointer;
                data[i].RowPitch = (nint)subresourceData.RowPitch;
                data[i].SlicePitch = (nint)subresourceData.SlicePitch;
            }

            D3D12UploadContext uploadContext = default;
            void* mappedData = null;
            if (description.CpuAccess == CpuAccessMode.Write)
            {
                mappedData = pMappedData;
            }
            else
            {
                uploadContext = _device.Allocate(allocatedSize);
                mappedData = uploadContext.UploadBuffer.pMappedData;
            }

            for (uint i = 0; i < numSubResources; ++i)
            {
                if (_rowSizesInBytes[i] > unchecked((ulong)-1))
                    continue;

                D3D12_MEMCPY_DEST DestData = new();
                DestData.pData = (void*)((ulong)mappedData + _footprints[i].Offset);
                DestData.RowPitch = _footprints[i].Footprint.RowPitch;
                DestData.SlicePitch = _footprints[i].Footprint.RowPitch * _numRows[i];
                MemcpySubresource(&DestData, &data[i], (nuint)_rowSizesInBytes[i], _numRows[i], _footprints[i].Footprint.Depth);

                if (uploadContext.IsValid)
                {
                    D3D12_TEXTURE_COPY_LOCATION Dst = new(_handle, i);
                    D3D12_TEXTURE_COPY_LOCATION Src = new(uploadContext.UploadBuffer!.Handle, _footprints[i]);
                    uploadContext.CommandList.Get()->CopyTextureRegion(
                        &Dst,
                        0,
                        0,
                        0,
                        &Src,
                        null
                    );
                }
            }

            if (uploadContext.IsValid)
            {
                _device.Submit(in uploadContext);
            }
        }

        State = description.InitialLayout;
    }

    public D3D12Texture(D3D12GraphicsDevice device, ID3D12Resource* existingTexture, in TextureDescription description)
        : base(description)
    {
        _device = device;
        DxgiFormat = (DXGI_FORMAT)description.Format.ToDxgiFormat();
        _handle = existingTexture;
        State = description.InitialLayout;

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public DXGI_FORMAT DxgiFormat { get; }
    public ID3D12Resource* Handle => _handle;
    public ResourceStates State { get; set; }
    public ResourceStates TransitioningState { get; set; } = (ResourceStates)uint.MaxValue;
    public ulong AllocatedSize { get; }

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
        Free(_footprints);
        Free(_rowSizesInBytes);
        Free(_numRows);
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
