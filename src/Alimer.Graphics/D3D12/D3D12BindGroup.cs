// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12;

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
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = default; // _device.GetResourceHeapCpuHandle(0);

            foreach (D3D12_DESCRIPTOR_RANGE1 range in _layout._cbvUavSrvDescriptorRanges)
            {
                switch (range.RangeType)
                {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        //device.Handle->CopyDescriptorsSimple(range.NumDescriptors, cpu_handle, device->nullCBV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                        for (uint index = 0; index < range.NumDescriptors; ++index)
                        {
                            uint shaderRegister = range.BaseShaderRegister + index;
                            ref readonly BindGroupEntry entry = ref description.Entries[shaderRegister];

                            if (entry.Buffer != null)
                            {
                                D3D12Buffer buffer = (D3D12Buffer)entry.Buffer;
                                ulong offset = entry.Offset;

                                D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = new();
                                cbvDesc.BufferLocation = buffer.GpuAddress + offset;
                                cbvDesc.SizeInBytes = (uint)MathHelper.AlignUp(entry.Size - offset, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
                                //cbvDesc.SizeInBytes = std::min(cbv.SizeInBytes, 65536u);

                                device.Handle->CreateConstantBufferView(&cbvDesc, cpuHandle);
                            }
                            cpuHandle.ptr += _device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                        }
                        break;
                }
            }
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

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
