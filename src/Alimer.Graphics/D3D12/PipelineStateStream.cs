// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_PIPELINE_STATE_SUBOBJECT_TYPE;

namespace Alimer.Graphics.D3D12;

internal struct AlignedSubObjectType<T> where T : unmanaged
{
    internal D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _type;
    internal T _inner;
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_FLAGS
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_PIPELINE_STATE_FLAGS> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_FLAGS(D3D12_PIPELINE_STATE_FLAGS flags)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS;
        _type._inner = flags;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_FLAGS(D3D12_PIPELINE_STATE_FLAGS flags) => new(flags);
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<uint> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK(uint nodeMask)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK;
        _type._inner = nodeMask;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK(uint nodeMask) => new(nodeMask);
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE
{
    public readonly D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
    public readonly ID3D12RootSignature* RootSignature;

    public CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(ID3D12RootSignature* rootSignature)
    {
        Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
        RootSignature = rootSignature;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(ID3D12RootSignature* rootSignature) => new(rootSignature);
}

[StructLayout(LayoutKind.Explicit)]
internal readonly unsafe struct CD3DX12_PIPELINE_STATE_STREAM_CS
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_CS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type._inner = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_CS(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type._inner = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_CS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}
