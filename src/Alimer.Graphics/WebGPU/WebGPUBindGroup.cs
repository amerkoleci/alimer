// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUBindGroup : BindGroup
{
    private readonly WebGPUGraphicsDevice _device;
    private readonly WebGPUBindGroupLayout _layout;

    public WebGPUBindGroup(WebGPUGraphicsDevice device, BindGroupLayout layout, in BindGroupDescription description)
        : base(description)
    {
        _device = device;
        _layout = (WebGPUBindGroupLayout)layout;

        WGPUBindGroupDescriptor descriptor = new()
        {
            layout = _layout.Handle,
            entryCount = (nuint)description.Entries.Length,
        };

        Handle = wgpuDeviceCreateBindGroup(device.Handle, &descriptor);
        if (Handle.IsNull)
        {
            Log.Error($"Vulkan: Failed to create {nameof(BindGroup)}.");
            return;
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUBindGroup" /> class.
    /// </summary>
    ~WebGPUBindGroup() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override BindGroupLayout Layout => _layout;

    public WGPUBindGroup Handle { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuBindGroupSetLabel(Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuBindGroupRelease(Handle);
    }
}
