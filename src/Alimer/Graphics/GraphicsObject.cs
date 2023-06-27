// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using Alimer.Graphics.VGPU;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// An base graphics object that was created by <see cref="GraphicsDevice"/>.
/// </summary>
public abstract class GraphicsObject : GraphicsObjectBase
{
    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public GraphicsDevice Device { get; }

    internal WeakReference<GraphicsObject?>? WeakReference;

    /// <summary>
    /// Initializes a new instance of the <see cref="GraphicsObject" /> class.
    /// </summary>
    /// <param name="device">The device object that created the object.</param>
    /// <param name="label">The label of the object or <c>null</c> to use <see cref="MemberInfo.Name" />.</param>
    protected GraphicsObject(GraphicsDevice device, bool trackResource = true, string? label = default)
        : base(label)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;

        if (trackResource)
        {
            WeakReference = new(this);
            Device.AddResourceReference(WeakReference);
        }
    }

    ~GraphicsObject()
    {
        // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
        Dispose(disposing: false);
    }

    protected override void Dispose(bool disposing)
    {
        if (WeakReference is not null)
        {
            Destroy();
            Device.RemoveResourceReference(WeakReference);
            WeakReference.SetTarget(null);
            WeakReference = null;
        }
    }

    protected abstract void Destroy();
}
