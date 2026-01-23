// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a <see cref="BindGroupLayout"/>.
/// </summary>
public struct BindGroupLayoutDescriptor
{
    public BindGroupLayoutEntry[] Entries;

    /// <summary>
    /// The label of <see cref="BindGroupLayout"/>.
    /// </summary>
    public string? Label;

    public BindGroupLayoutDescriptor()
    {
        Entries = [];
    }

    public BindGroupLayoutDescriptor(params BindGroupLayoutEntry[] entries)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        Entries = entries;
    }

}
