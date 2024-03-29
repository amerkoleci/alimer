// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// An base graphics object that was created by <see cref="GraphicsDevice"/>.
/// </summary>
public abstract class GraphicsObject : GraphicsObjectBase
{
    /// <summary>
    /// Initializes a new instance of the <see cref="GraphicsObject" /> class.
    /// </summary>
    /// <param name="label">The label of the object or <c>null</c> to use <see cref="MemberInfo.Name" />.</param>
    protected GraphicsObject(string? label = default)
        : base(label)
    {
    }

    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Device.QueueDestroy(this);
        }
    }

    /// <summary>
    /// The safe moment to actually destroy object.
    /// </summary>
    protected internal abstract void Destroy();
}
