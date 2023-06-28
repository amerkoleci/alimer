// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Reflection;

namespace Alimer.Graphics;

/// <summary>
/// Base class for graphics objects, that implements <see cref="IDisposable"/> interface.
/// </summary>
public abstract class GraphicsObjectBase : DisposableObject
{
    private string? _label;

    /// <summary>
    /// Initializes a new instance of the <see cref="GraphicsObjectBase" /> class.
    /// </summary>
    /// <param name="label">The label of the object or <c>null</c>.</param>
    protected GraphicsObjectBase(string? label = default)
    {
        _label = label;
    }

    /// <summary>
    /// Gets or sets the label that identifies this object.
    /// </summary>
    public string? Label
    {
        get => _label;
        set
        {
            _label = value;
            OnLabelChanged(_label ?? string.Empty);
        }
    }

    protected virtual void OnLabelChanged(string newLabel)
    {
    }

    /// <inheritdoc />
    public override string? ToString()
    {
        if (string.IsNullOrEmpty(_label))
        {
            return base.ToString();
        }

        return _label;
    }
}
