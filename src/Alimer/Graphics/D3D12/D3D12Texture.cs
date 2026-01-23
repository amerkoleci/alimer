// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Alimer.Utilities.MemoryUtilities;
using static TerraFX.Interop.DirectX.D3D12_DSV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_HEAP_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RTV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_LAYOUT;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.Windows.E;
using static TerraFX.Interop.Windows.Windows;
using DescriptorIndex = System.UInt32;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Texture : Texture, ID3D12GpuResource
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12Resource> _handle;
    private readonly nint _allocation;
    private readonly HANDLE _sharedHandle = HANDLE.NULL;
    private readonly D3D12_PLACED_SUBRESOURCE_FOOTPRINT* _footprints;
    private readonly ulong* _rowSizesInBytes;
    private readonly uint* _numRows;
    private readonly void* pMappedData = default;

    private readonly Dictionary<int, DescriptorIndex> _RTVs = [];
    private readonly Dictionary<int, DescriptorIndex> _DSVs = [];

    public D3D12Texture(D3D12GraphicsDevice device, in TextureDescriptor description, TextureData* initialData)
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

        TextureLayout initialLayout = TextureLayout.Undefined;
        if (initialData == null)
        {
            if ((description.Usage & TextureUsage.RenderTarget) != 0)
            {
                if (isDepthStencil)
                {
                    initialLayout = TextureLayout.DepthWrite;
                }
                else
                {
                    initialLayout = TextureLayout.RenderTarget;
                }
            }
            else if ((description.Usage & TextureUsage.ShaderWrite) != 0)
            {
                initialLayout = TextureLayout.UnorderedAccess;
            }
            else if ((description.Usage & TextureUsage.ShaderRead) != 0)
            {
                initialLayout = TextureLayout.ShaderResource;
            }
        }

        D3D12_RESOURCE_DESC1 resourceDesc = new()
        {
            Dimension = description.Dimension.ToD3D12(),
            Alignment = 0,
            Width = description.Width,
            Height = description.Height,
            DepthOrArraySize = (ushort)description.DepthOrArrayLayers,
            MipLevels = (ushort)description.MipLevelCount,
            Format = DxgiFormat,
            SampleDesc = new(description.SampleCount.ToSampleCount(), 0),
            Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            Flags = resourceFlags
        };

        ulong allocatedSize = 0;
        uint numSubResources = Math.Max(1u, resourceDesc.MipLevels) * resourceDesc.DepthOrArraySize;
        _footprints = AllocateArray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>(numSubResources);
        _rowSizesInBytes = AllocateArray<ulong>(numSubResources);
        _numRows = AllocateArray<uint>(numSubResources);
        if (device.EnhancedBarriersSupported)
        {
            device.Device8->GetCopyableFootprints1(
                &resourceDesc,
                0,
                numSubResources,
                0,
                _footprints,
                _numRows,
                _rowSizesInBytes,
                &allocatedSize
            );
        }
        else
        {
            device.Device->GetCopyableFootprints(
                (D3D12_RESOURCE_DESC*)&resourceDesc,
                0,
                numSubResources,
                0,
                _footprints,
                _numRows,
                _rowSizesInBytes,
                &allocatedSize
            );
        }

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

        D3D12MA.ALLOCATION_DESC allocationDesc = new()
        {
            HeapType = D3D12_HEAP_TYPE_DEFAULT
        };

        HRESULT hr = E_FAIL;
        if (device.EnhancedBarriersSupported)
        {
            D3D12TextureLayoutMapping textureLayout = ConvertTextureLayout(initialLayout);

            hr = D3D12MA.Allocator_CreateResource3(
                device.MemoryAllocator,
                &allocationDesc,
                &resourceDesc,
                textureLayout.Layout,
                null,
                0, null,
                out _allocation,
                __uuidof<ID3D12Resource>(), (void**)_handle.GetAddressOf()
            );
        }
        else
        {
            D3D12_RESOURCE_STATES initialStateLegacy = ConvertTextureLayoutLegacy(initialLayout);

            hr = D3D12MA.Allocator_CreateResource2(
                device.MemoryAllocator,
                &allocationDesc,
                &resourceDesc,
                initialStateLegacy,
                pClearValue,
                out _allocation,
                __uuidof<ID3D12Resource>(), (void**)_handle.GetAddressOf()
                );
        }

        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Texture.");
            return;
        }

        if (isShared)
        {
            HANDLE sharedHandle = default;
            if (device.Device->CreateSharedHandle((ID3D12DeviceChild*)_handle.Get(), null, GENERIC_ALL, null, &sharedHandle).FAILED)
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
            if (description.MemoryType == MemoryType.Upload)
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

        SetTextureLayout(initialLayout);
    }

    public D3D12Texture(D3D12GraphicsDevice device, ID3D12Resource* existingTexture, in TextureDescriptor description, TextureLayout initialLayout)
        : base(description)
    {
        _device = device;
        DxgiFormat = (DXGI_FORMAT)description.Format.ToDxgiFormat();
        _handle = existingTexture;

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        SetTextureLayout(initialLayout);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public DXGI_FORMAT DxgiFormat { get; }
    public ID3D12Resource* Handle => _handle;
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

        if (_allocation != 0)
        {
            _ = D3D12MA.Allocation_Release(_allocation);
        }

        _handle.Dispose();
        Free(_footprints);
        Free(_rowSizesInBytes);
        Free(_numRows);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _handle.Get()->SetName(newLabel);
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint mipSlice, uint arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
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
            _device.Device->CreateRenderTargetView(_handle.Get(), &viewDesc, cpuHandle);
        }

        return _device.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(uint mipSlice, uint arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
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
            _device.Device->CreateDepthStencilView(_handle.Get(), &viewDesc, cpuHandle);
        }

        return _device.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
    }
}
