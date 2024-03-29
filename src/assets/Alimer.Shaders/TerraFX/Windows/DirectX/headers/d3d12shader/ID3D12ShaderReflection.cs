// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from d3d12shader.h in microsoft/DirectX-Headers tag v1.606.4
// Original source is Copyright © Microsoft. Licensed under the MIT license

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.IID;

#pragma warning disable CS0649

namespace TerraFX.Interop.DirectX;

[Guid("5A58797D-A72C-478D-8BA2-EFC6B0EFE88E")]
[NativeTypeName("struct ID3D12ShaderReflection : IUnknown")]
[NativeInheritance("IUnknown")]
internal unsafe partial struct ID3D12ShaderReflection : ID3D12ShaderReflection.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(in IID_ID3D12ShaderReflection));

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(0)]
    public HRESULT QueryInterface([NativeTypeName("const IID &")] Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, Guid*, void**, int>)(lpVtbl[0]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(1)]
    [return: NativeTypeName("ULONG")]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[1]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(2)]
    [return: NativeTypeName("ULONG")]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[2]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(3)]
    public HRESULT GetDesc(D3D12_SHADER_DESC* pDesc)
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, D3D12_SHADER_DESC*, int>)(lpVtbl[3]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), pDesc);
    }

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(4)]
    //public ID3D12ShaderReflectionConstantBuffer* GetConstantBufferByIndex(uint Index)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint, ID3D12ShaderReflectionConstantBuffer*>)(lpVtbl[4]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), Index);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(5)]
    //public ID3D12ShaderReflectionConstantBuffer* GetConstantBufferByName([NativeTypeName("LPCSTR")] sbyte* Name)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, sbyte*, ID3D12ShaderReflectionConstantBuffer*>)(lpVtbl[5]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), Name);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(6)]
    //public HRESULT GetResourceBindingDesc(uint ResourceIndex, D3D12_SHADER_INPUT_BIND_DESC* pDesc)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint, D3D12_SHADER_INPUT_BIND_DESC*, int>)(lpVtbl[6]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), ResourceIndex, pDesc);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(7)]
    //public HRESULT GetInputParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint, D3D12_SIGNATURE_PARAMETER_DESC*, int>)(lpVtbl[7]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), ParameterIndex, pDesc);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(8)]
    //public HRESULT GetOutputParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint, D3D12_SIGNATURE_PARAMETER_DESC*, int>)(lpVtbl[8]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), ParameterIndex, pDesc);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(9)]
    //public HRESULT GetPatchConstantParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint, D3D12_SIGNATURE_PARAMETER_DESC*, int>)(lpVtbl[9]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), ParameterIndex, pDesc);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(10)]
    //public ID3D12ShaderReflectionVariable* GetVariableByName([NativeTypeName("LPCSTR")] sbyte* Name)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, sbyte*, ID3D12ShaderReflectionVariable*>)(lpVtbl[10]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), Name);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(11)]
    //public HRESULT GetResourceBindingDescByName([NativeTypeName("LPCSTR")] sbyte* Name, D3D12_SHADER_INPUT_BIND_DESC* pDesc)
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, sbyte*, D3D12_SHADER_INPUT_BIND_DESC*, int>)(lpVtbl[11]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), Name, pDesc);
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(12)]
    //public uint GetMovInstructionCount()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[12]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(13)]
    //public uint GetMovcInstructionCount()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[13]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(14)]
    //public uint GetConversionInstructionCount()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[14]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(15)]
    //public uint GetBitwiseInstructionCount()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[15]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(16)]
    //public D3D_PRIMITIVE GetGSInputPrimitive()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, D3D_PRIMITIVE>)(lpVtbl[16]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(17)]
    //public BOOL IsSampleFrequencyShader()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, int>)(lpVtbl[17]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(18)]
    //public uint GetNumInterfaceSlots()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint>)(lpVtbl[18]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(19)]
    public HRESULT GetMinFeatureLevel([NativeTypeName("enum D3D_FEATURE_LEVEL *")] D3D_FEATURE_LEVEL* pLevel)
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, D3D_FEATURE_LEVEL*, int>)(lpVtbl[19]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), pLevel);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(20)]
    public uint GetThreadGroupSize(uint* pSizeX, uint* pSizeY, uint* pSizeZ)
    {
        return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, uint*, uint*, uint*, uint>)(lpVtbl[20]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this), pSizeX, pSizeY, pSizeZ);
    }

    //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[VtblIndex(21)]
    //[return: NativeTypeName("UINT64")]
    //public ulong GetRequiresFlags()
    //{
    //    return ((delegate* unmanaged[MemberFunction]<ID3D12ShaderReflection*, ulong>)(lpVtbl[21]))((ID3D12ShaderReflection*)Unsafe.AsPointer(ref this));
    //}

    public interface Interface : IUnknown.Interface
    {
        [VtblIndex(3)]
        HRESULT GetDesc(D3D12_SHADER_DESC* pDesc);

        //[VtblIndex(4)]
        //ID3D12ShaderReflectionConstantBuffer* GetConstantBufferByIndex(uint Index);

        //[VtblIndex(5)]
        //ID3D12ShaderReflectionConstantBuffer* GetConstantBufferByName([NativeTypeName("LPCSTR")] sbyte* Name);

        //[VtblIndex(6)]
        //HRESULT GetResourceBindingDesc(uint ResourceIndex, D3D12_SHADER_INPUT_BIND_DESC* pDesc);

        //[VtblIndex(7)]
        //HRESULT GetInputParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc);

        //[VtblIndex(8)]
        //HRESULT GetOutputParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc);

        //[VtblIndex(9)]
        //HRESULT GetPatchConstantParameterDesc(uint ParameterIndex, D3D12_SIGNATURE_PARAMETER_DESC* pDesc);

        //[VtblIndex(10)]
        //ID3D12ShaderReflectionVariable* GetVariableByName([NativeTypeName("LPCSTR")] sbyte* Name);

        //[VtblIndex(11)]
        //HRESULT GetResourceBindingDescByName([NativeTypeName("LPCSTR")] sbyte* Name, D3D12_SHADER_INPUT_BIND_DESC* pDesc);

        //[VtblIndex(12)]
        //uint GetMovInstructionCount();

        //[VtblIndex(13)]
        //uint GetMovcInstructionCount();

        //[VtblIndex(14)]
        //uint GetConversionInstructionCount();

        //[VtblIndex(15)]
        //uint GetBitwiseInstructionCount();

        //[VtblIndex(16)]
        //D3D_PRIMITIVE GetGSInputPrimitive();

        //[VtblIndex(17)]
        //BOOL IsSampleFrequencyShader();

        //[VtblIndex(18)]
        //uint GetNumInterfaceSlots();

        [VtblIndex(19)]
        HRESULT GetMinFeatureLevel([NativeTypeName("enum D3D_FEATURE_LEVEL *")] D3D_FEATURE_LEVEL* pLevel);

        [VtblIndex(20)]
        uint GetThreadGroupSize(uint* pSizeX, uint* pSizeY, uint* pSizeZ);

        //[VtblIndex(21)]
        //[return: NativeTypeName("UINT64")]
        //ulong GetRequiresFlags();
    }
}
