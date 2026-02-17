// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_SRV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_UAV_DIMENSION;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_BUFFER_SRV_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_BUFFER_UAV_FLAGS;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindGroup : BindGroup
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12BindGroupLayout _layout;

    public D3D12BindGroup(D3D12BindGroupLayout layout, in BindGroupDescriptor descriptor)
        : base(descriptor)
    {
        _device = layout.DXDevice;
        _layout = layout;

        // Update descriptor entries
        Update(descriptor.Entries);
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12BindGroup" /> class.
    /// </summary>
    ~D3D12BindGroup() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override BindGroupLayout Layout => _layout;

    public uint DescriptorTableCbvUavSrv { get; private set; }

    public uint DescriptorTableSamplers { get; private set; }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (DescriptorTableCbvUavSrv > 0)
        {
            _device.ShaderResourceViewHeap.ReleaseDescriptors(DescriptorTableCbvUavSrv, _layout.DescriptorTableSizeCbvUavSrv);
        }

        if (DescriptorTableSamplers > 0)
        {
            _device.SamplerHeap.ReleaseDescriptors(DescriptorTableSamplers, _layout.DescriptorTableSizeSamplers);
        }
    }

    /// <inheitdoc />
    public override void Update(Span<BindGroupEntry> entries)
    {
        // Free existing descriptors
        Destroy();

        if (_layout.DescriptorTableSizeCbvUavSrv > 0)
        {
            uint descriptorTableBaseIndex = _device.ShaderResourceViewHeap.AllocateDescriptors(_layout.DescriptorTableSizeCbvUavSrv);
            DescriptorTableCbvUavSrv = descriptorTableBaseIndex;

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout.CbvUavSrvDescriptorRanges)
            {
                for (uint itemInRange = 0; itemInRange < range.NumDescriptors; ++itemInRange)
                {
                    uint slot = range.BaseShaderRegister + itemInRange;

                    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _device.ShaderResourceViewHeap.GetCpuHandle(descriptorTableBaseIndex + range.OffsetInDescriptorsFromTableStart + itemInRange);

                    // Find the layout entry for this binding to determine expected type
                    bool foundLayoutEntry = false;
                    BindGroupLayoutEntry layoutEntry = default;
                    for (int layoutEntryIndex = 0; layoutEntryIndex < _layout.Entries.Length; layoutEntryIndex++)
                    {
                        ref BindGroupLayoutEntry iterLayoutEntry = ref _layout.Entries[layoutEntryIndex];

                        D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType = iterLayoutEntry.ToD3D12RangeType();
                        if (range.RangeType == descriptorRangeType && iterLayoutEntry.Binding == slot)
                        {
                            layoutEntry = iterLayoutEntry;
                            foundLayoutEntry = true;
                            break;
                        }
                    }

                    if (!foundLayoutEntry)
                    {
                        throw new InvalidOperationException("Layout entry should have a defined binding type");
                    }

                    bool found = false;
                    foreach (BindGroupEntry entry in entries)
                    {
                        if (entry.Binding + entry.ArrayElement != slot)
                            continue;

                        if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
                        {
                            // Verify the layout entry expects an SRV (texture or read-only storage buffer)
                            bool isTexture = layoutEntry.BindingType == BindingInfoType.Texture;
                            if (isTexture && entry.Resource is GraphicsBuffer)
                                continue;

                            bool isShaderReadBuffer = layoutEntry.BindingType == BindingInfoType.Buffer && layoutEntry.Buffer.Type == BufferBindingType.ShaderRead;

                            if (!isTexture && !isShaderReadBuffer)
                                continue;

                            if (entry.Resource is GraphicsBuffer buffer)
                            {
                                D3D12Buffer backendBuffer = (D3D12Buffer)buffer;
                                ulong offset = entry.Offset;
                                ulong size = entry.Size == WholeSize ? buffer.Size : entry.Size;

                                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = new();
                                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                                uint stride = 0; // backendBuffer->GetStride();
                                if (stride == 0)
                                {
                                    // Raw Buffer (ByteAddressBuffer in HLSL) -> WebGPU
                                    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                                    srvDesc.Buffer.FirstElement = offset / sizeof(uint);
                                    srvDesc.Buffer.NumElements = (uint)size / sizeof(uint);
                                    srvDesc.Buffer.StructureByteStride = 0;
                                    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
                                }
                                else
                                {
                                    // structured buffer offset must be aligned to structure stride!
                                    Debug.Assert(MathUtilities.IsAligned(offset, stride));

                                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                                    srvDesc.Buffer.FirstElement = offset / stride;
                                    srvDesc.Buffer.NumElements = (uint)((size - offset) / stride);
                                    srvDesc.Buffer.StructureByteStride = (uint)stride;
                                    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                                }

                                _device.Device->CreateShaderResourceView(backendBuffer.Handle, &srvDesc, descriptorHandle);
                                found = true;
                                break;
                            }
                            else if (entry.Resource is D3D12TextureView backendTextureView)
                            {
                                D3D12Texture backendTexture = (D3D12Texture)backendTextureView.Texture;
                                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = backendTextureView.GetSRVDescriptor();
                                _device.Device->CreateShaderResourceView(backendTexture.Handle, &viewDesc, descriptorHandle);

                                found = true;
                                break;
                            }
                            else if (entry.Resource is D3D12Texture backendTexture)
                            {
                                D3D12TextureView backendTextureDefaultView = (D3D12TextureView)backendTexture.DefaultView!;
                                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = backendTextureDefaultView.GetSRVDescriptor();
                                _device.Device->CreateShaderResourceView(backendTexture.Handle, &viewDesc, descriptorHandle);

                                found = true;
                                break;
                            }
                        }
                        else if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
                        {
                            // Verify the layout entry expects a UAV (storage buffer or storage texture)
                            bool isStorageTexture = layoutEntry.StorageTexture.Access != StorageTextureAccess.Undefined;
                            bool isStorageBuffer = layoutEntry.BindingType == BindingInfoType.Buffer && layoutEntry.Buffer.Type == BufferBindingType.ShaderReadWrite;

                            if (!isStorageTexture && !isStorageBuffer)
                                continue;

                            if (entry.Resource is D3D12Buffer backendBuffer)
                            {
                                ulong offset = entry.Offset;
                                ulong size = entry.Size == WholeSize ? backendBuffer.Size : entry.Size;

                                // TODO: TypedBuffer_UAV
                                D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = new();
                                viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                                viewDesc.Format = DXGI_FORMAT_UNKNOWN;

                                uint stride = 0; // backendBuffer->GetStride();
                                if (stride > 0)
                                {
                                    // structured buffer offset must be aligned to structure stride!
                                    Debug.Assert(MathUtilities.IsAligned(offset, stride));

                                    ulong firstElement = offset / stride;
                                    uint numElements = (uint)((size - offset) / stride);

                                    viewDesc.Buffer.FirstElement = offset / stride;
                                    viewDesc.Buffer.NumElements = (uint)((size - offset) / stride);
                                    viewDesc.Buffer.StructureByteStride = (uint)stride;
                                    viewDesc.Buffer.CounterOffsetInBytes = 0;
                                    viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

                                }
                                else if (viewDesc.Format != DXGI_FORMAT_UNKNOWN)
                                {
                                    // Typed buffer
                                    //uint32_t stride = GetFormatStride(format);
                                    //viewDesc.Format = _ConvertFormat(format);
                                    //viewDesc.Buffer.FirstElement = UINT(offset / stride);
                                    //viewDesc.Buffer.Buffer.NumElements = UINT(std::min(size, desc.size - offset) / stride);
                                    //viewDesc.Buffer.StructureByteStride = (uint)stride;
                                    //viewDesc.Buffer.CounterOffsetInBytes = 0;
                                    //viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                                }
                                else
                                {
                                    // Raw Buffer (ByteAddressBuffer in HLSL) -> WebGPU
                                    viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                                    viewDesc.Buffer.FirstElement = offset / sizeof(uint);
                                    viewDesc.Buffer.NumElements = (uint)(size / sizeof(uint));
                                    viewDesc.Buffer.StructureByteStride = 0;
                                    viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
                                }

                                _device.Device->CreateUnorderedAccessView(backendBuffer.Handle, null, &viewDesc, descriptorHandle);
                                found = true;
                                break;
                            }
                            else if (entry.Resource is D3D12TextureView backendTextureView)
                            {
                                D3D12Texture backendTexture = (D3D12Texture)backendTextureView.Texture;

                                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = backendTextureView.GetUAVDescriptor();
                                _device.Device->CreateUnorderedAccessView(backendTexture.Handle, null, &uavDesc, descriptorHandle);
                                found = true;
                                break;
                            }
                            else if (entry.Resource is D3D12Texture backendTexture)
                            {
                                D3D12TextureView backendTextureDefaultView = (D3D12TextureView)backendTexture.DefaultView!;
                                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = backendTextureDefaultView.GetUAVDescriptor();
                                _device.Device->CreateUnorderedAccessView(backendTexture.Handle, null, &uavDesc, descriptorHandle);
                                found = true;
                                break;
                            }
                        }
                        else if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
                        {
                            // Only process if this is a buffer suitable for CBV
                            if (entry.Resource is not D3D12Buffer backendBuffer)
                                continue;

                            // Verify the layout entry expects a uniform buffer
                            bool isUniformBuffer = layoutEntry.BindingType == BindingInfoType.Buffer && layoutEntry.Buffer.Type == BufferBindingType.Constant;

                            if (!isUniformBuffer)
                                continue;

                            ulong offset = entry.Offset;
                            ulong size = entry.Size == WholeSize ? backendBuffer.Size : entry.Size;

                            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = new()
                            {
                                BufferLocation = backendBuffer.GpuAddress + offset,
                                SizeInBytes = (uint)MathUtilities.AlignUp(size - offset, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
                            };
                            _device.Device->CreateConstantBufferView(&cbvDesc, descriptorHandle);
                            found = true;
                            break;

                        }
                    }

                    if (!found)
                    {
                        // Create a null SRV, UAV, or CBV
                        switch (range.RangeType)
                        {
                            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                                CreateNullSRV(descriptorHandle);
                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                                CreateNullUAV(descriptorHandle);
                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                                _device.Device->CreateConstantBufferView(null, descriptorHandle);
                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                            default:
                                ThrowHelper.ThrowArgumentException(nameof(range.RangeType), "Invalid range type");
                                break;
                        }
                    }
                }
            }

            _device.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorTableBaseIndex, _layout.DescriptorTableSizeCbvUavSrv);
        }

        // Sampler heap
        if (_layout.DescriptorTableSizeSamplers > 0)
        {
            uint descriptorTableBaseIndex = _device.SamplerHeap.AllocateDescriptors(_layout.DescriptorTableSizeSamplers);
            DescriptorTableSamplers = descriptorTableBaseIndex;

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout.SamplerDescriptorRanges)
            {
                for (uint itemInRange = 0; itemInRange < range.NumDescriptors; ++itemInRange)
                {
                    uint slot = range.BaseShaderRegister + itemInRange;

                    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _device.SamplerHeap.GetCpuHandle(descriptorTableBaseIndex + range.OffsetInDescriptorsFromTableStart + itemInRange);

                    bool found = false;
                    foreach (BindGroupEntry entry in entries)
                    {
                        if (entry.Binding + entry.ArrayElement == slot && entry.Resource is Sampler sampler)
                        {
                            D3D12Sampler backendSampler = (D3D12Sampler)sampler;
                            backendSampler.CreateDescriptor(descriptorHandle);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        // Create a default sampler
                        D3D12_SAMPLER_DESC samplerDesc = new();
                        _device.Device->CreateSampler(&samplerDesc, descriptorHandle);
                        continue;
                    }
                }
            }

            _device.SamplerHeap.CopyToShaderVisibleHeap(descriptorTableBaseIndex, _layout.DescriptorTableSizeSamplers);
        }
    }

    public void CreateNullSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, DXGI_FORMAT srvFormat = DXGI_FORMAT_R32_UINT)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = new()
        {
            Format = srvFormat,
            ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
        };
        _device.Device->CreateShaderResourceView(null, &viewDesc, descriptor);
    }

    public void CreateNullUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, DXGI_FORMAT srvFormat = DXGI_FORMAT_R32_UINT)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = new()
        {
            Format = srvFormat,
            ViewDimension = D3D12_UAV_DIMENSION_BUFFER
        };
        _device.Device->CreateUnorderedAccessView(null, null, &viewDesc, descriptor);
    }
}
