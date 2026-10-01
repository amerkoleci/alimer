// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics.Metal;

internal unsafe sealed class MetalBuffer : GraphicsBuffer
{
    private readonly MetalGraphicsDevice _device;
    private readonly void* _mappedData;

    public MetalBuffer(MetalGraphicsDevice device, in GraphicsBufferDescriptor descriptor, void* initialData)
        : base(descriptor)
    {
        _device = device;

        _mappedData = NativeMemory.Alloc((nuint)descriptor.Size);
        if (initialData != null)
        {
            NativeMemory.Copy(initialData, _mappedData, (nuint)descriptor.Size);
        }

        GpuAddress = 0u;
    }

    public override GraphicsDevice Device => _device;

    public override GPUAddress GpuAddress { get; }

    public override void* GetMappedData() => _mappedData;

    protected override GraphicsBufferView CreateViewCore(in GraphicsBufferViewDescriptor descriptor)
        => new MetalBufferView(this, in descriptor);

    protected internal override void Destroy()
    {
        if (_mappedData != null)
        {
            NativeMemory.Free(_mappedData);
        }
    }
}
