// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;
using Alimer.Numerics;
using System.Runtime.CompilerServices;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUBuffer : GraphicsBuffer
{
    private readonly WebGPUGraphicsDevice _device;
    private void* pMappedData;
    private readonly ulong _mappedSize;

    public WebGPUBuffer(WebGPUGraphicsDevice device, in BufferDescription description, void* initialData)
        : base(description)
    {
        _device = device;

        WGPUBufferDescriptor descriptor = new()
        {
            size = description.Size,
            usage = WGPUBufferUsage.None
        };

        if ((description.Usage & BufferUsage.Vertex) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Vertex;
        }

        if ((description.Usage & BufferUsage.Index) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Index;
        }

        if ((description.Usage & BufferUsage.Constant) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Uniform;
        }

        if ((description.Usage & BufferUsage.ShaderRead) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Storage;
        }
        if ((description.Usage & BufferUsage.ShaderWrite) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Storage;
        }
        if ((description.Usage & BufferUsage.Indirect) != 0)
        {
            descriptor.usage |= WGPUBufferUsage.Indirect;
        }

        //if ((description.Usage & BufferUsage.Predication) != 0 &&
        //    device.QueryFeatureSupport(Feature.Predication))
        //{
        //    descriptor.usage |= WGPUBufferUsage.ConditionalRenderingEXT;
        //}
        //
        //if ((description.Usage & BufferUsage.RayTracing) != 0 &&
        //    device.QueryFeatureSupport(Feature.RayTracing))
        //{
        //    descriptor.usage |= WGPUBufferUsage.RayTracing;
        //}

        if (description.CpuAccess == CpuAccessMode.Read)
        {
            descriptor.usage |= WGPUBufferUsage.MapRead;
            descriptor.usage |= WGPUBufferUsage.CopyDst;
        }
        else if (description.CpuAccess == CpuAccessMode.Write)
        {
            descriptor.usage |= WGPUBufferUsage.MapWrite;
            descriptor.usage |= WGPUBufferUsage.CopySrc;
        }
        else
        {
            descriptor.usage |= WGPUBufferUsage.CopyDst;
        }

        Handle = wgpuDeviceCreateBuffer(device.Handle, &descriptor);

        if (Handle.IsNull)
        {
            Log.Error("WebGPU: Failed to create buffer.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        void OnMapCallback(WGPUBufferMapAsyncStatus status, nint userdata = 0)
        {
            if (status != WGPUBufferMapAsyncStatus.Success)
                return;

            pMappedData = (void*)wgpuBufferGetMappedRange(Handle, 0, (nuint)Size);
            //wgpuBufferUnmap(Handle);
        };

        if (description.CpuAccess == CpuAccessMode.Read || description.CpuAccess == CpuAccessMode.Write)
        {
            _mappedSize = description.Size;
            wgpuBufferMapAsync(Handle, WGPUMapMode.Read, 0, (nuint)description.Size, OnMapCallback, 0);
        }

        // Issue data copy on request
        if (initialData != null)
        {
            wgpuQueueWriteBuffer(_device.GraphicsQueue.Handle, Handle, 0, initialData, (nuint)description.Size);
        }
    }

    public WebGPUBuffer(WebGPUGraphicsDevice device, WGPUBuffer existingHandle, in BufferDescription descriptor)
        : base(descriptor)
    {
        _device = device;
        Handle = existingHandle;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public WGPUBuffer Handle { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUBuffer" /> class.
    /// </summary>
    ~WebGPUBuffer() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuBufferSetLabel(Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuBufferReference(Handle);
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
