// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using Win32;
using Win32.Graphics.Direct3D11;
using static Alimer.Graphics.D3D11.D3D11Utils;
using D3D11Usage = Win32.Graphics.Direct3D11.Usage;
using System.Diagnostics;
using Win32.Graphics.Dxgi;

namespace Alimer.Graphics.D3D11;

internal unsafe class D3D11Buffer : GraphicsBuffer
{
    private readonly ComPtr<ID3D11Buffer> _handle;

    public D3D11Buffer(D3D11GraphicsDevice device, in BufferDescriptor descriptor, void* initialData)
        : base(device, descriptor)
    {
        uint alignment = BufferSizeAlignment(descriptor.Usage);

        BufferDescription d3dDesc = new()
        {
            ByteWidth = MathHelper.AlignUp((uint)descriptor.Size, alignment),
            Usage = D3D11Usage.Default,
            BindFlags = BindFlags.None,
            CPUAccessFlags = CpuAccessFlags.None,
            MiscFlags = ResourceMiscFlags.None
        };

        // TODO: Handle Constant combined with other flags (see: https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/d3d11/BufferD3D11.cpp)
        if ((descriptor.Usage & BufferUsage.Constant) != 0)
        {
            d3dDesc.BindFlags = BindFlags.ConstantBuffer;
            d3dDesc.Usage = D3D11Usage.Dynamic;
            d3dDesc.CPUAccessFlags = CpuAccessFlags.Write;
        }
        else
        {
            switch (descriptor.CpuAccess)
            {
                case CpuAccessMode.None:
                    d3dDesc.Usage = D3D11Usage.Default;
                    d3dDesc.CPUAccessFlags = 0;
                    break;

                case CpuAccessMode.Read:
                    d3dDesc.Usage = D3D11Usage.Staging;
                    d3dDesc.CPUAccessFlags = CpuAccessFlags.Read;
                    break;

                case CpuAccessMode.Write:
                    d3dDesc.Usage = D3D11Usage.Dynamic;
                    d3dDesc.CPUAccessFlags = CpuAccessFlags.Write;
                    break;
            }
        }

        bool byteAddressBuffer = false;
        bool structuredBuffer = false;

        if ((descriptor.Usage & BufferUsage.Vertex) != 0)
        {
            d3dDesc.BindFlags |= BindFlags.VertexBuffer;
        }

        if ((descriptor.Usage & BufferUsage.Index) != 0)
        {
            d3dDesc.BindFlags |= BindFlags.IndexBuffer;
        }

        if ((descriptor.Usage & BufferUsage.ShaderRead) != 0)
        {
            d3dDesc.BindFlags |= BindFlags.ShaderResource;
            //byteAddressBuffer = true;
        }

        // UAV buffers cannot be dynamic
        if ((descriptor.Usage & BufferUsage.ShaderWrite) != 0)
        {
            Debug.Assert(d3dDesc.Usage != D3D11Usage.Dynamic);
            d3dDesc.BindFlags |= BindFlags.UnorderedAccess;
            byteAddressBuffer = true;
        }

        if ((descriptor.Usage & BufferUsage.Indirect) != 0)
        {
            d3dDesc.MiscFlags |= ResourceMiscFlags.DrawIndirectArgs;
        }

        if (byteAddressBuffer)
        {
            d3dDesc.MiscFlags |= ResourceMiscFlags.BufferAllowRawViews;
        }
        else if (structuredBuffer)
        {
            //bufferDesc.StructureByteStride = desc->stride;
            d3dDesc.MiscFlags |= ResourceMiscFlags.BufferStructured;
        }

        SubresourceData initData = default;
        if (initialData != null)
        {
            initData.pSysMem = initialData;
        }

        HResult hr = device.Handle->CreateBuffer(&d3dDesc, initialData != null ? &initData : default, _handle.GetAddressOf());

        if (hr.Failure)
        {
            //return nullptr;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    public D3D11Buffer(D3D11GraphicsDevice device, ID3D11Buffer* existingHandle, in BufferDescriptor descriptor)
        : base(device, descriptor)
    {
        _handle = existingHandle;
    }

    public ID3D11Buffer* Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="D3D11Buffer" /> class.
    /// </summary>
    ~D3D11Buffer() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        _handle.Dispose();
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _handle.Get()->SetDebugName(newLabel);
    }
}
