// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D12_SRV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_UAV_DIMENSION;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_BUFFER_SRV_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_BUFFER_UAV_FLAGS;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_LAYOUT;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.Windows.Windows;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12BufferView : GPUBufferView
{
    private int _bindlessReadIndex = InvalidBindlessIndex;
    private int _bindlessReadWriteIndex = InvalidBindlessIndex;

    public D3D12BufferView(D3D12Buffer buffer, in GPUBufferViewDescriptor descriptor)
        : base(buffer, descriptor)
    {
        bool shaderRead = (buffer.Usage & GPUBufferUsage.ShaderRead) != 0;
        bool shaderReadWrite = (buffer.Usage & GPUBufferUsage.ShaderWrite) != 0;

        if (shaderRead)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = new();
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.FirstElement = descriptor.ElementOffset;
            srvDesc.Buffer.NumElements = descriptor.ElementCount;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

            if (descriptor.ElementFormat == PixelFormat.Undefined)
            {
                // StructuredBuffer in HLSL
                srvDesc.Buffer.StructureByteStride = descriptor.ElementSize;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            }
            else if (descriptor.ElementFormat == PixelFormat.R32Uint)
            {
                // Raw Buffer (ByteAddressBuffer in HLSL) -> WebGPU
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.Buffer.StructureByteStride = 0;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            }
            else
            {
                // This is a Typed Buffer
                PixelFormat viewFormat = descriptor.ElementFormat.SrgbToLinearFormat();
                srvDesc.Format = viewFormat.ToDxgiSRVFormat();
            }

            _bindlessReadIndex = buffer.DXDevice.BindlessManager.AllocateSRV(buffer.Handle, srvDesc);
        }

        if (shaderReadWrite)
        {

        }
    }

    /// <inheritdoc />
    public override int BindlessReadIndex => _bindlessReadIndex;

    /// <inheritdoc />
    public override int BindlessReadWriteIndex => _bindlessReadWriteIndex;

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        if (_bindlessReadIndex != InvalidBindlessIndex)
        {
            ((D3D12Buffer)Buffer).DXDevice.BindlessManager.FreeSRV(_bindlessReadIndex);
        }

        if (_bindlessReadWriteIndex != InvalidBindlessIndex)
        {
            ((D3D12Buffer)Buffer).DXDevice.BindlessManager.FreeUAV(_bindlessReadWriteIndex);
        }

        _bindlessReadIndex = InvalidBindlessIndex;
        _bindlessReadWriteIndex = InvalidBindlessIndex;
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
    }
}
