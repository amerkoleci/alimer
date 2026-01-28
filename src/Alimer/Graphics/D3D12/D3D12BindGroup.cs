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
using static Alimer.Graphics.Constants;

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
    public override void Update(ReadOnlySpan<BindGroupEntry> entries)
    {
        // Free existing descriptors
        Destroy();

        if (_layout.DescriptorTableSizeCbvUavSrv > 0)
        {
            uint descriptorTableBaseIndex = _device.ShaderResourceViewHeap.AllocateDescriptors(_layout.DescriptorTableSizeCbvUavSrv);
            DescriptorTableCbvUavSrv = descriptorTableBaseIndex;

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout.CbvUavSrvDescriptorRanges)
            {
                for (uint index = 0; index < range.NumDescriptors; ++index)
                {
                    uint binding = range.BaseShaderRegister + index;

                    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _device.ShaderResourceViewHeap.GetCpuHandle(
                        descriptorTableBaseIndex + range.OffsetInDescriptorsFromTableStart + index);

                    bool found = false;
                    foreach (BindGroupEntry entry in entries)
                    {
                        if (entry.Binding != binding)
                            continue;

                        if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV && entry.TextureView != null)
                        {
                            //if (entry.Buffer != null)
                            //{
                            //    D3D12Buffer buffer = (D3D12Buffer)entry.Buffer;
                            //    ulong offset = entry.Offset;
                            //
                            //    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = new()
                            //    {
                            //        BufferLocation = buffer.GpuAddress + offset,
                            //        SizeInBytes = (uint)MathHelper.AlignUp(entry.Size - offset, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
                            //    };
                            //    device.Handle->CreateConstantBufferView(&cbvDesc, descriptorHandle);
                            //}
                            //else
                            {
                                D3D12TextureView backendTextureView = (D3D12TextureView)entry.TextureView!;
                                D3D12Texture backendTexture = (D3D12Texture)backendTextureView.Texture;
                                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = backendTextureView.GetSRV();
                                _device.Device->CreateShaderResourceView(backendTexture.Handle, &viewDesc, descriptorHandle);
                            }

                            found = true;
                            break;
                        }
                        else if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV && entry.Buffer != null)
                        {
                            D3D12Buffer buffer = (D3D12Buffer)entry.Buffer;
                            ulong offset = entry.Offset;
                            ulong size = entry.Size == WholeSize ? entry.Buffer.Size : entry.Size;

                            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = new()
                            {
                                BufferLocation = buffer.GpuAddress + offset,
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

        if (_layout.DescriptorTableSizeSamplers > 0)
        {
            uint descriptorTableBaseIndex = _device.SamplerHeap.AllocateDescriptors(_layout.DescriptorTableSizeSamplers);
            DescriptorTableSamplers = descriptorTableBaseIndex;

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout.SamplerDescriptorRanges)
            {
                for (uint index = 0; index < range.NumDescriptors; ++index)
                {
                    uint binding = range.BaseShaderRegister + index;

                    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _device.SamplerHeap.GetCpuHandle(descriptorTableBaseIndex + range.OffsetInDescriptorsFromTableStart + index);

                    bool found = false;
                    foreach (BindGroupEntry entry in entries)
                    {
                        if (entry.Binding == binding && entry.Sampler != null)
                        {
                            D3D12Sampler backendSampler = (D3D12Sampler)entry.Sampler;
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
