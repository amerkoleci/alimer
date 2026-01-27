// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract class BindGroupLayout : GraphicsObject
{
    protected BindGroupLayout(in BindGroupLayoutDescriptor descriptor)
        : base(descriptor.Label)
    {
    }

    public BindGroup CreateBindGroup(in BindGroupDescriptor descriptor)
    {
        Guard.IsNotEmpty(descriptor.Entries, nameof(BindGroupDescriptor.Entries));
        Guard.IsGreaterThan(descriptor.Entries.Length, 0, nameof(BindGroupDescriptor.Entries));

        return Device.CreateBindGroup(this, in descriptor);
    }

    public BindGroup CreateBindGroup(params ReadOnlySpan<BindGroupEntry> entries)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        return Device.CreateBindGroup(this, new BindGroupDescriptor(entries));
    }
}
