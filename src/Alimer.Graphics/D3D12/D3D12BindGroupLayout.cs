// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_FLAGS;
using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BindGroupLayout : BindGroupLayout
{
    private readonly D3D12GraphicsDevice _device;
    public readonly List<D3D12_DESCRIPTOR_RANGE1> CbvUavSrvDescriptorRanges = new();
    public readonly List<D3D12_DESCRIPTOR_RANGE1> SamplerDescriptorRanges = new();
    public readonly List<D3D12_STATIC_SAMPLER_DESC> StaticSamplers = new();

    public D3D12BindGroupLayout(D3D12GraphicsDevice device, in BindGroupLayoutDescription description)
        : base(description)
    {
        _device = device;

        int bindingCount = description.Entries.Length;

        D3D12_DESCRIPTOR_RANGE_TYPE currentType = (D3D12_DESCRIPTOR_RANGE_TYPE)int.MaxValue;
        uint currentBinding = ~0u;

        for (int i = 0; i < bindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];
            if (entry.StaticSampler.HasValue)
            {
                D3D12_STATIC_SAMPLER_DESC statiSamplerDesc = D3D12Utils.ToD3D12StaticSamplerDesc(
                    entry.Binding,
                    entry.StaticSampler.Value,
                    entry.Visibility.ToD3D12(),
                    D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START);
                StaticSamplers.Add(statiSamplerDesc);
                continue;
            }

            D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType = entry.BindingType.ToD3D12();
            if ((descriptorRangeType != currentType) || currentBinding == ~0u || entry.Binding != currentBinding + 1)
            {
                // Start a new range
                D3D12_DESCRIPTOR_RANGE1 range = new()
                {
                    RangeType = descriptorRangeType,
                    NumDescriptors = 1,
                    BaseShaderRegister = entry.Binding,
                    RegisterSpace = D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START,
                    OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
                };

                switch (descriptorRangeType)
                {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                        range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
                        break;

                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
                        break;

                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                        range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                        break;
                }

                currentType = descriptorRangeType;
                currentBinding = entry.Binding;

                if (descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                {
                    range.OffsetInDescriptorsFromTableStart = DescriptorTableSizeSamplers;
                    SamplerDescriptorRanges.Add(range);

                    DescriptorTableSizeSamplers++;
                }
                else
                {
                    range.OffsetInDescriptorsFromTableStart = DescriptorTableSizeCbvUavSrv;
                    CbvUavSrvDescriptorRanges.Add(range);

                    DescriptorTableSizeCbvUavSrv++;
                }
            }
            else
            {
                // Extend the current range
                if (descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                {
                    Debug.Assert(SamplerDescriptorRanges.Count > 0);

                    D3D12_DESCRIPTOR_RANGE1 range = SamplerDescriptorRanges[SamplerDescriptorRanges.Count - 1];
                    range.NumDescriptors += 1;
                    SamplerDescriptorRanges[SamplerDescriptorRanges.Count - 1] = range;

                    DescriptorTableSizeSamplers++;
                }
                else
                {
                    Debug.Assert(CbvUavSrvDescriptorRanges.Count > 0);

                    D3D12_DESCRIPTOR_RANGE1 range = CbvUavSrvDescriptorRanges[CbvUavSrvDescriptorRanges.Count - 1];
                    range.NumDescriptors += 1;
                    CbvUavSrvDescriptorRanges[CbvUavSrvDescriptorRanges.Count - 1] = range;

                    DescriptorTableSizeCbvUavSrv++;
                }

                currentBinding = entry.Binding;
            }
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12BindGroupLayout" /> class.
    /// </summary>
    ~D3D12BindGroupLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public uint DescriptorTableSizeCbvUavSrv = 0;
    public uint DescriptorTableSizeSamplers = 0;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
