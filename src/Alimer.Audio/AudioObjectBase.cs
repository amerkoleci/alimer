// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Base class for audio objects, that implements <see cref="IDisposable"/> interface.
/// </summary>
public abstract class AudioObjectBase : DisposableObject
{
    private string? _label;

    /// <summary>
    /// Initializes a new instance of the <see cref="AudioObjectBase" /> class.
    /// </summary>
    /// <param name="label">The label of the object or <c>null</c>.</param>
    protected AudioObjectBase(string? label = default)
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

    /// <inheritdoc />
    public override string? ToString()
    {
        if (string.IsNullOrEmpty(_label))
        {
            return base.ToString();
        }

        return _label;
    }

    protected virtual void OnLabelChanged(string newLabel)
    {
    }
}
