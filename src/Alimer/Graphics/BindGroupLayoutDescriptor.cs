// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a <see cref="BindGroupLayout"/>.
/// </summary>
public ref struct BindGroupLayoutDescriptor
{
    public ReadOnlySpan<BindGroupLayoutEntry> Entries = [];

    /// <summary>
    /// The label of <see cref="BindGroupLayout"/>.
    /// </summary>
    public string? Label;

    public BindGroupLayoutDescriptor()
    {
    }

    public BindGroupLayoutDescriptor(ReadOnlySpan<BindGroupLayoutEntry> entries)
    {
        Guard.IsGreaterThan(entries.Length, 0, nameof(entries));

        Entries = entries;
    }
}
