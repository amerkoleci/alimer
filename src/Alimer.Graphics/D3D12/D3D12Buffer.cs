// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using Win32;
using Win32.Graphics.Direct3D12;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static Win32.Graphics.Direct3D12.Apis;
using static Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Buffer : GraphicsBuffer
{
    private readonly ComPtr<ID3D12Resource> _handle;

    public D3D12Buffer(D3D12GraphicsDevice device, in BufferDescriptor descriptor, void* initialData)
        : base(device, descriptor)
    {
        ulong alignedSize = descriptor.Size;
        if ((descriptor.Usage & BufferUsage.Constant) != 0)
        {
            alignedSize = MathHelper.AlignUp(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }

        ResourceFlags resourceFlags = ResourceFlags.None;
        if ((descriptor.Usage & BufferUsage.ShaderWrite) != 0)
        {
            resourceFlags |= ResourceFlags.AllowUnorderedAccess;
        }

        if ((descriptor.Usage & BufferUsage.ShaderRead) == 0 &&
            (descriptor.Usage & BufferUsage.RayTracing) == 0)
        {
            resourceFlags |= ResourceFlags.DenyShaderResource;
        }

        ResourceDescription resourceDesc = ResourceDescription.Buffer(alignedSize, resourceFlags);
        ResourceStates initialState = ResourceStates.Common;
        HeapProperties heapProps = DefaultHeapProps;
        if (descriptor.CpuAccess == CpuAccessMode.Read)
        {
            heapProps = ReadbackHeapProps;
            initialState = ResourceStates.CopyDest;
            resourceDesc.Flags |= ResourceFlags.DenyShaderResource;
        }
        else if (descriptor.CpuAccess == CpuAccessMode.Write)
        {
            heapProps = UploadHeapProps;
            initialState = ResourceStates.GenericRead;
        }
        else
        {
            //initialState = ConvertResourceStates(desc.initialState);
        }

        HResult hr = device.Handle->CreateCommittedResource(&heapProps,
            HeapFlags.None,
            &resourceDesc,
            initialState,
            null,
            __uuidof<ID3D12Resource>(), _handle.GetVoidAddressOf()
            );

        if (hr.Failure)
        {
            //return nullptr;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public D3D12Buffer(GraphicsDevice device, ID3D12Resource* existingHandle, in BufferDescriptor descriptor)
        : base(device, descriptor)
    {
        _handle = existingHandle;
    }

    public ID3D12Resource* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Buffer" /> class.
    /// </summary>
    ~D3D12Buffer() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        fixed (char* pName = newLabel)
        {
            _ = _handle.Get()->SetName((ushort*)pName);
        }
    }
}
