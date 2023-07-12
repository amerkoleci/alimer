// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_PIPELINE_STATE_SUBOBJECT_TYPE;

namespace Alimer.Graphics.D3D12;

internal unsafe interface IPipelineStreamObject
{
    protected internal static abstract D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type { get; }
}

internal struct AlignedSubObjectType<T> where T : unmanaged
{
    public D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
    public T Value;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;

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
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_VS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_VS()
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
    }

    public unsafe CD3DX12_PIPELINE_STATE_STREAM_VS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
        _type.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_VS(D3D12_SHADER_BYTECODE byteCode)
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
        _type.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_VS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_PS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_PS()
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
    }

    public unsafe CD3DX12_PIPELINE_STATE_STREAM_PS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
        _type.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_PS(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
        _type.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_PS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_HS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_HS()
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS;
    }

    public unsafe CD3DX12_PIPELINE_STATE_STREAM_HS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS;
        _type.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_HS(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS;
        _type.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_HS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_DS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_DS()
    {
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS;
    }

    public unsafe CD3DX12_PIPELINE_STATE_STREAM_DS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS;
        _type.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_DS(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS;
        _type.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_DS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly unsafe struct CD3DX12_PIPELINE_STATE_STREAM_CS
{
    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_CS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_CS(D3D12_SHADER_BYTECODE byteCode)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
        _type.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_CS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_BLEND_DESC> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC(D3D12_BLEND_DESC desc)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
        _type.Value = desc;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC(D3D12_BLEND_DESC desc)
    {
        return new(desc);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<uint> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK()
        : this(uint.MaxValue)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK(uint value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK(uint value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_RASTERIZER_DESC> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER()
        : this(D3D12_RASTERIZER_DESC.DEFAULT)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(D3D12_RASTERIZER_DESC value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(D3D12_RASTERIZER_DESC value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_INPUT_LAYOUT_DESC> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT(D3D12_INPUT_LAYOUT_DESC value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT(D3D12_INPUT_LAYOUT_DESC value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_PRIMITIVE_TOPOLOGY_TYPE> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(D3D12_PRIMITIVE_TOPOLOGY_TYPE desc)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
        _type.Value = desc;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(D3D12_PRIMITIVE_TOPOLOGY_TYPE value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_RT_FORMAT_ARRAY> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(D3D12_RT_FORMAT_ARRAY value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(D3D12_RT_FORMAT_ARRAY value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<DXGI_FORMAT> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(DXGI_FORMAT value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(DXGI_FORMAT value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<DXGI_SAMPLE_DESC> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC()
        : this(new DXGI_SAMPLE_DESC(1, 0))
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC(uint count, uint quality)
        : this(new DXGI_SAMPLE_DESC(count, quality))
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC(DXGI_SAMPLE_DESC value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC(DXGI_SAMPLE_DESC value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<uint> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK()
        : this(0)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK(uint nodeMask)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK;
        _type.Value = nodeMask;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK(uint nodeMask) => new(nodeMask);
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
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS;
        _type.Value = flags;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_FLAGS(D3D12_PIPELINE_STATE_FLAGS flags) => new(flags);
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_DEPTH_STENCIL_DESC1> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1()
        : this(D3D12_DEPTH_STENCIL_DESC1.DEFAULT)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1(D3D12_DEPTH_STENCIL_DESC1 value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1(D3D12_DEPTH_STENCIL_DESC1 value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING
{
    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_VIEW_INSTANCING_DESC> _type;

    [FieldOffset(0)]
    internal readonly nint _pad;

    public CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING()
        : this(D3D12_VIEW_INSTANCING_DESC.DEFAULT)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING(D3D12_VIEW_INSTANCING_DESC value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING(D3D12_VIEW_INSTANCING_DESC value)
    {
        return new(value);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly unsafe struct CD3DX12_PIPELINE_STATE_STREAM_AS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    public readonly D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;

    [FieldOffset(4)]
    public readonly D3D12_SHADER_BYTECODE Value;

    public CD3DX12_PIPELINE_STATE_STREAM_AS()
    {
        Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;
    }

    public CD3DX12_PIPELINE_STATE_STREAM_AS(void* pShaderBytecode, nuint bytecodeLength)
    {
        Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;
        Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_AS(D3D12_SHADER_BYTECODE byteCode)
    {
        Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;
        Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_AS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly unsafe struct CD3DX12_PIPELINE_STATE_STREAM_MS : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_SHADER_BYTECODE> _inner;

    public CD3DX12_PIPELINE_STATE_STREAM_MS()
    {
        _inner.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
    }

    public CD3DX12_PIPELINE_STATE_STREAM_MS(void* pShaderBytecode, nuint bytecodeLength)
    {
        _inner.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
        _inner.Value = new D3D12_SHADER_BYTECODE(pShaderBytecode, bytecodeLength);
    }

    public CD3DX12_PIPELINE_STATE_STREAM_MS(D3D12_SHADER_BYTECODE byteCode)
    {
        _inner.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
        _inner.Value = byteCode;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_MS(D3D12_SHADER_BYTECODE byteCode)
    {
        return new(byteCode);
    }
}

[StructLayout(LayoutKind.Explicit)]
internal readonly struct CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL2 : IPipelineStreamObject
{
    static D3D12_PIPELINE_STATE_SUBOBJECT_TYPE IPipelineStreamObject.Type => D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL2;

    [FieldOffset(0)]
    private readonly nint _pad;

    [FieldOffset(0)]
    internal readonly AlignedSubObjectType<D3D12_DEPTH_STENCIL_DESC2> _type;

    public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL2()
        : this(D3D12_DEPTH_STENCIL_DESC2.DEFAULT)
    {
    }

    public CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL2(D3D12_DEPTH_STENCIL_DESC2 value)
    {
        _pad = default;
        _type.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL2;
        _type.Value = value;
    }

    public static implicit operator CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL2(D3D12_DEPTH_STENCIL_DESC2 value)
    {
        return new(value);
    }
}
