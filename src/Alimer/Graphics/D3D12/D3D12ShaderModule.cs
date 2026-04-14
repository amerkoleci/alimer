// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12ShaderModule : ShaderModule
{
    private readonly D3D12GraphicsDevice _device;
    private readonly void* _byteCodePtr;
    private readonly D3D12_SHADER_BYTECODE _byteCode;

    public D3D12ShaderModule(D3D12GraphicsDevice device, in ShaderModuleDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        _byteCodePtr = NativeMemory.Alloc((uint)descriptor.ByteCode.Length);
        descriptor.ByteCode.CopyTo(new(_byteCodePtr, descriptor.ByteCode.Length));
        _byteCode = new D3D12_SHADER_BYTECODE(_byteCodePtr, (uint)descriptor.ByteCode.Length);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ref readonly D3D12_SHADER_BYTECODE ByteCode => ref _byteCode;

    protected internal override void Destroy()
    {
        NativeMemory.Free(_byteCodePtr);
    }
}
