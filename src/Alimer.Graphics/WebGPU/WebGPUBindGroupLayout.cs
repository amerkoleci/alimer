// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Utilities.MemoryUtilities;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUBindGroupLayout : BindGroupLayout
{
    private readonly WebGPUGraphicsDevice _device;

    public WebGPUBindGroupLayout(WebGPUGraphicsDevice device, in BindGroupLayoutDescription description)
        : base(description)
    {
        _device = device;

        int bindingCount = description.Entries.Length;
        WGPUBindGroupLayoutEntry* entries = stackalloc WGPUBindGroupLayoutEntry[bindingCount];

        for (int i = 0; i < bindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];

            // This needs to map with ShaderCompiler
            const uint constantBuffer = 0;
            const uint shaderResource = 100;
            const uint unorderedAccess = 200;
            const uint sampler = 300;

            uint registerOffset = 0;

            entries[i] = new WGPUBindGroupLayoutEntry
            {
                visibility = entry.Visibility.ToWebGPU()
            };

            switch (entry.BindingType)
            {
                case BindingInfoType.Buffer:
                    entries[i].buffer = new WGPUBufferBindingLayout()
                    {
                        type = WGPUBufferBindingType.Uniform,
                        hasDynamicOffset = entry.Buffer.HasDynamicOffset,
                        minBindingSize = entry.Buffer.MinBindingSize
                    };

                    registerOffset = constantBuffer;
                    break;

                case BindingInfoType.Sampler:
                    entries[i].sampler = new WGPUSamplerBindingLayout()
                    {
                        type = WGPUSamplerBindingType.Filtering
                    };

                    registerOffset = sampler;
                    break;

                case BindingInfoType.Texture:
                    entries[i].texture = new WGPUTextureBindingLayout()
                    {
                        sampleType = WGPUTextureSampleType.Float,
                        viewDimension = WGPUTextureViewDimension._2D,
                    };
                    registerOffset = shaderResource;
                    break;

                case BindingInfoType.StorageTexture:
                    entries[i].storageTexture = new WGPUStorageTextureBindingLayout
                    {
                        access = WGPUStorageTextureAccess.Undefined,
                        format = WGPUTextureFormat.Undefined,
                        viewDimension = WGPUTextureViewDimension._2D,
                    };

                    registerOffset = unorderedAccess;
                    break;

                default:
                    ThrowHelper.ThrowInvalidOperationException();
                    break;
            }

            entries[i].binding = entry.Binding + registerOffset;
        }

        WGPUBindGroupLayoutDescriptor descriptor = new()
        {
            entryCount = (nuint)bindingCount,
            entries = entries
        };

        Handle = wgpuDeviceCreateBindGroupLayout(device.Handle, &descriptor);
        if (Handle.IsNull)
        {
            Log.Error($"WebGPU: Failed to create {nameof(BindGroupLayout)}.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUBindGroupLayout" /> class.
    /// </summary>
    ~WebGPUBindGroupLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public WGPUBindGroupLayout Handle { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuBindGroupLayoutSetLabel(Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuBindGroupLayoutRelease(Handle);
    }
}
