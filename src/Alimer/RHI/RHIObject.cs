// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.RHI;

/// <summary>
/// An base graphics object that was created by <see cref="GraphicsDevice"/>.
/// </summary>
/// <remarks>
/// Initializes a new instance of the <see cref="RHIObject" /> class.
/// </remarks>
/// <param name="label">The label of the object or <c>null</c>.</param>
public abstract class RHIObject(string? label = default) : DisposableObject
{
    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    /// <summary>
    /// Gets or sets the label that identifies this object.
    /// </summary>
    public string? Label
    {
        get => label;
        set
        {
            if (label == value)
                return;

            label = value;
            OnLabelChanged(value);
        }
    }

    protected virtual void OnLabelChanged(string? newLabel)
    {
    }

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
