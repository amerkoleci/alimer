// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using Vortice.Mathematics;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Buffer : GraphicsBuffer, ID3D12GpuResource
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<D3D12MA_Allocation> _allocation;
    private readonly ComPtr<ID3D12Resource> _handle;
    public readonly void* pMappedData;

    public D3D12Buffer(D3D12GraphicsDevice device, in BufferDescription description, void* initialData)
        : base(description)
    {
        _device = device;

        ulong alignedSize = description.Size;
        if ((description.Usage & BufferUsage.Constant) != 0)
        {
            alignedSize = MathHelper.AlignUp(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }

        D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
        if ((description.Usage & BufferUsage.ShaderWrite) != 0)
        {
            resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        if ((description.Usage & BufferUsage.ShaderRead) == 0 &&
            (description.Usage & BufferUsage.RayTracing) == 0)
        {
            resourceFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        D3D12_RESOURCE_DESC resourceDesc = D3D12_RESOURCE_DESC.Buffer(alignedSize, resourceFlags);
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

        D3D12MA_ALLOCATION_DESC allocationDesc = new();
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        if (description.CpuAccess == CpuAccessMode.Read)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            initialState = D3D12_RESOURCE_STATE_COPY_DEST;
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        else if (description.CpuAccess == CpuAccessMode.Write)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else
        {
            //initialState = ConvertResourceStates(desc.initialState);
        }

        HRESULT hr = device.MemoryAllocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            initialState,
            null,
            _allocation.GetAddressOf(),
            __uuidof<ID3D12Resource>(), _handle.GetVoidAddressOf()
            );

        if (hr.FAILED)
        {
            Log.Error("D3D12: Failed to create Buffer.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        ulong allocatedSize;
        device.Handle->GetCopyableFootprints(&resourceDesc, 0, 1, 0, null, null, null, &allocatedSize);
        GpuAddress = _handle.Get()->GetGPUVirtualAddress();
        AllocatedSize = allocatedSize;

        if (description.CpuAccess == CpuAccessMode.Read)
        {
            void* mappedData;
            ThrowIfFailed(_handle.Get()->Map(0, null, &mappedData));
            pMappedData = mappedData;
        }
        else if (description.CpuAccess == CpuAccessMode.Write)
        {
            D3D12_RANGE readRange = default;
            void* mappedData;
            ThrowIfFailed(_handle.Get()->Map(0, &readRange, &mappedData));
            pMappedData = mappedData;
        }

        // Issue data copy on request
        if (initialData != null)
        {
            D3D12UploadContext context = default;
            void* mappedData = null;
            if (description.CpuAccess == CpuAccessMode.Write)
            {
                mappedData = pMappedData;
            }
            else
            {
                context = device.Allocate(description.Size);
                mappedData = context.UploadBuffer.pMappedData;
            }

            Unsafe.CopyBlockUnaligned(mappedData, initialData, (uint)description.Size);
            //std::memcpy(mappedData, initialData, desc.size);

            if (context.IsValid)
            {
                context.CommandList.Get()->CopyBufferRegion(
                    _handle.Get(),
                    0,
                    context.UploadBuffer!.Handle,
                    0,
                    description.Size
                );

                device.Submit(in context);
            }
        }
    }

    public D3D12Buffer(D3D12GraphicsDevice device, ID3D12Resource* existingHandle, in BufferDescription descriptor)
        : base(descriptor)
    {
        _device = device;
        _handle = existingHandle;
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D12Buffer" /> class.
    /// </summary>
    ~D3D12Buffer() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public ID3D12Resource* Handle => _handle;
    public ResourceStates State { get; set; }
    public ResourceStates TransitioningState { get; set; } = (ResourceStates)uint.MaxValue;

    public ulong GpuAddress { get; }
    public ulong AllocatedSize { get; }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _allocation.Dispose();
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

    /// <inheitdoc />
    protected override void SetDataUnsafe(void* dataPtr, int offsetInBytes)
    {
        Unsafe.CopyBlockUnaligned((byte*)pMappedData + offsetInBytes, dataPtr, (uint)Size);
    }

    /// <inheitdoc />
    protected override void GetDataUnsafe(void* destPtr, int offsetInBytes)
    {
        Unsafe.CopyBlockUnaligned(destPtr, (byte*)pMappedData + offsetInBytes, (uint)Size);
    }
}
