// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_LAYOUT;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.Windows.E;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12Buffer : GraphicsBuffer, ID3D12GpuResource
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12Resource> _handle;
    private readonly nint _allocation;
    public readonly void* pMappedData;

    public D3D12Buffer(D3D12GraphicsDevice device, in BufferDescription description, void* initialData)
        : base(description)
    {
        _device = device;

        ulong alignedSize = description.Size;
        if ((description.Usage & BufferUsage.Constant) != 0)
        {
            alignedSize = MathUtilities.AlignUp(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
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

        D3D12_RESOURCE_DESC1 resourceDesc = D3D12_RESOURCE_DESC1.Buffer(alignedSize, resourceFlags);

        D3D12MA.ALLOCATION_DESC allocationDesc = new()
        {
            HeapType = D3D12_HEAP_TYPE_DEFAULT
        };

        // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
        // Buffers may only use D3D12_BARRIER_LAYOUT_UNDEFINED as an initial layout.
        D3D12_BARRIER_LAYOUT initialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
        D3D12_RESOURCE_STATES initialStateLegacy = D3D12_RESOURCE_STATE_COMMON;

        if (description.CpuAccess == CpuAccessMode.Read)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

            initialStateLegacy = D3D12_RESOURCE_STATE_COPY_DEST;
            ImmutableState = true;
        }
        else if (description.CpuAccess == CpuAccessMode.Write)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            initialStateLegacy = D3D12_RESOURCE_STATE_GENERIC_READ;
            ImmutableState = true;
        }

        HRESULT hr = E_FAIL;
        if (device.EnhancedBarriersSupported)
        {
            hr = D3D12MA.Allocator_CreateResource3(
                device.MemoryAllocator,
                &allocationDesc,
                &resourceDesc,
                initialLayout,
                null,
                0, null,
                out _allocation,
                __uuidof<ID3D12Resource>(), (void**)_handle.GetAddressOf()
            );
        }
        else
        {
            hr = D3D12MA.Allocator_CreateResource2(
                device.MemoryAllocator,
                &allocationDesc,
                &resourceDesc,
                initialStateLegacy,
                null,
                out _allocation,
                __uuidof<ID3D12Resource>(), (void**)_handle.GetAddressOf()
            );
        }

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
        if (device.EnhancedBarriersSupported)
        {
            device.Device8->GetCopyableFootprints1(&resourceDesc,
                0, 1, 0, null, null, null, &allocatedSize);
        }
        else
        {
            device.Device->GetCopyableFootprints((D3D12_RESOURCE_DESC*)&resourceDesc,
                0, 1, 0, null, null, null, &allocatedSize);
        }

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
    public bool ImmutableState { get; }

    public ulong GpuAddress { get; }
    public ulong AllocatedSize { get; }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _ = D3D12MA.Allocation_Release(_allocation);
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
         _handle.Get()->SetName(newLabel);
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
