// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_SRV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_UAV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindGroup : BindGroup
{
    private readonly D3D12GraphicsDevice _device;
    private readonly D3D12BindGroupLayout _layout;

    public D3D12BindGroup(D3D12GraphicsDevice device, BindGroupLayout layout, in BindGroupDescription description)
        : base(description)
    {
        _device = device;
        _layout = (D3D12BindGroupLayout)layout;

        if (_layout._cbvUavSrvDescriptorRanges.Count > 0)
        {
            uint descriptorTableBaseIndex = _device.ShaderResourceViewHeap.AllocateDescriptors(_layout.DescriptorTableSizeCbvUavSrv);
            DescriptorTableCbvUavSrv = descriptorTableBaseIndex;

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout._cbvUavSrvDescriptorRanges)
            {
                //device.Handle->CopyDescriptorsSimple(range.NumDescriptors, cpu_handle, device->nullCBV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                for (uint index = 0; index < range.NumDescriptors; ++index)
                {
                    uint shaderRegister = range.BaseShaderRegister + index;

                    D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _device.ShaderResourceViewHeap.GetCpuHandle(
                        descriptorTableBaseIndex/* + range.OffsetInDescriptorsFromTableStart*/ + index);

                    if (shaderRegister > description.Entries.Length - 1)
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
                                device.Handle->CreateConstantBufferView(null, descriptorHandle);
                                break;

                            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                            default:
                                ThrowHelper.ThrowArgumentException(nameof(range.RangeType), "Invalid range type");
                                break;
                        }

                        continue;
                    }

                    ref readonly BindGroupEntry entry = ref description.Entries[shaderRegister];

                    switch (range.RangeType)
                    {
                        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                            if (entry.Buffer != null)
                            {
                                D3D12Buffer buffer = (D3D12Buffer)entry.Buffer;
                                ulong offset = entry.Offset;

                                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = new()
                                {
                                    BufferLocation = buffer.GpuAddress + offset,
                                    SizeInBytes = (uint)MathHelper.AlignUp(entry.Size - offset, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
                                };
                                device.Handle->CreateConstantBufferView(&cbvDesc, descriptorHandle);
                            }
                            break;
                    }
                }
            }

            device.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorTableBaseIndex, _layout.DescriptorTableSizeCbvUavSrv);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12BindGroup" /> class.
    /// </summary>
    ~D3D12BindGroup() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override BindGroupLayout Layout => _layout;

    public uint DescriptorTableCbvUavSrv { get; }
    public uint DescriptorTableSamplers { get; }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _device.ShaderResourceViewHeap.ReleaseDescriptors(DescriptorTableCbvUavSrv, _layout.DescriptorTableSizeCbvUavSrv);
        _device.SamplerHeap.ReleaseDescriptors(DescriptorTableSamplers, _layout.DescriptorTableSizeSamplers);
    }

    public void CreateNullSRV(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, DXGI_FORMAT srvFormat = DXGI_FORMAT_R32_UINT)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = new()
        {
            Format = srvFormat,
            ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
        };
        _device.Handle->CreateShaderResourceView(null, &viewDesc, descriptor);
    }

    public void CreateNullUAV(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, DXGI_FORMAT srvFormat = DXGI_FORMAT_R32_UINT)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = new()
        {
            Format = srvFormat,
            ViewDimension = D3D12_UAV_DIMENSION_BUFFER
        };
        _device.Handle->CreateUnorderedAccessView(null, null, &viewDesc, descriptor);
    }
}
