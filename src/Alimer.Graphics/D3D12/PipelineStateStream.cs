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

[StructLayout(LayoutKind.Sequential)]
public unsafe readonly struct PipelineStateSubObjectTypeRootSignature
{
    public readonly D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
    public readonly ID3D12RootSignature* RootSignature;

    public PipelineStateSubObjectTypeRootSignature(ID3D12RootSignature* rootSignature)
    {
        Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
        RootSignature = rootSignature;
    }

    public static implicit operator PipelineStateSubObjectTypeRootSignature(ID3D12RootSignature* rootSignature) => new(rootSignature);
}

[StructLayout(LayoutKind.Explicit)]
public readonly unsafe struct PipelineStateSubObjectTypeComputeShader
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public PipelineStateSubObjectTypeComputeShader(ReadOnlySpan<byte> byteCode)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        fixed (byte* sourcePtr = byteCode)
        {
            _type._inner = new D3D12_SHADER_BYTECODE()
            {
                pShaderBytecode = sourcePtr,
                BytecodeLength = (nuint)byteCode.Length
            };
        }
    }

    public PipelineStateSubObjectTypeComputeShader(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type._inner = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public PipelineStateSubObjectTypeComputeShader(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type._type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type._inner = byteCode;
    }

    public static implicit operator PipelineStateSubObjectTypeComputeShader(ReadOnlySpan<byte> byteCode)
    {
        return new(byteCode);
    }

    public static implicit operator PipelineStateSubObjectTypeComputeShader(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}
