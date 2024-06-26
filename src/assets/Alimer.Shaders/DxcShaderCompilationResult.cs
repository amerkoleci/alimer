// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;

namespace Alimer.Shaders;

internal unsafe class DxcShaderCompilationResult : ShaderCompilationResult
{
    private ComPtr<IDxcBlob> _byteCode;

    public DxcShaderCompilationResult(string error)
        : base(error)
    {
    }

    public DxcShaderCompilationResult(ComPtr<IDxcBlob> byteCode, ShaderFormat format)
    {
        _byteCode = byteCode.Move();
        _byteCode.Get()->AddRef();
        Format = format;
    }

    public override void Dispose()
    {
        _byteCode.Dispose();
    }

    public override ShaderFormat Format { get; }

    public override ReadOnlySpan<byte> GetByteCode()
    {
        IDxcBlob* ptr = _byteCode.Get();
        if (ptr == null || ptr->GetBufferPointer() == null || ptr->GetBufferSize() == 0)
        {
            return [];
        }

        return new ReadOnlySpan<byte>(ptr->GetBufferPointer(), (int)ptr->GetBufferSize());
    }
}
