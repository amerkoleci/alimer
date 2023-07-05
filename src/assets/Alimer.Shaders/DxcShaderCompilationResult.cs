// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Win32;
using Win32.Graphics.Direct3D.Dxc;

namespace Alimer.Shaders;

internal unsafe class DxcShaderCompilationResult : ShaderCompilationResult
{
    private ComPtr<IDxcBlob> _byteCode;

    public DxcShaderCompilationResult(string error)
        : base(error)
    {
    }

    public DxcShaderCompilationResult(ComPtr<IDxcBlob> byteCode)
    {
        _byteCode = byteCode.Move();
    }

    public override void Dispose()
    {
        _byteCode.Dispose();
    }

    public override ReadOnlySpan<byte> GetByteCode()
    {
        IDxcBlob* ptr = _byteCode.Get();
        if (ptr == null || ptr->GetBufferPointer() == null || ptr->GetBufferSize() == 0)
        {
            return ReadOnlySpan<byte>.Empty;
        }

        return new ReadOnlySpan<byte>(ptr->GetBufferPointer(), (int)ptr->GetBufferSize());
    }
}
