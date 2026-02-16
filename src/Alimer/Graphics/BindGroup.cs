// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class BindGroup : GraphicsObject
{
    protected BindGroup(in BindGroupDescriptor descriptor)
        : base(descriptor.Label)
    {
    }

    /// <summary>
    /// Get the <see cref="BindGroupLayout"/>.
    /// </summary>
    public abstract BindGroupLayout Layout { get; }

    /// <summary>
    /// Updates the current set of resource bindings using the specified collection of bind group entries.
    /// </summary>
    /// <param name="entries">
    /// A read-only span containing the bind group entries to apply. Each entry specifies a resource to bind; the span
    /// must not be empty.
    /// </param>
    public abstract void Update(Span<BindGroupEntry> entries);
}
