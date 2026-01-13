// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using static Alimer.Numerics.MathUtilities;

namespace Alimer.UI;

partial class UIElement
{
    private bool _measuring;
    private Size? _previousMeasure;
    private Rect? _previousArrange;

    /// <summary>
    /// Gets the size that this element computed during the measure pass of the layout process.
    /// </summary>
    public Size DesiredSize
    {
        get;
        private set;
    }

    /// <summary>
    /// Gets a value indicating whether the control's layout measure is valid.
    /// </summary>
    public bool IsMeasureValid
    {
        get;
        private set;
    }

    /// <summary>
    /// Gets a value indicating whether the control's layouts arrange is valid.
    /// </summary>
    public bool IsArrangeValid
    {
        get;
        private set;
    }

    /// <summary>
    /// Invalidates the measurement of the control and queues a new layout pass.
    /// </summary>
    public void InvalidateMeasure()
    {
        if (!IsMeasureValid)
            return;

        IsMeasureValid = false;
        IsArrangeValid = false;

        //LayoutManager?.InvalidateMeasure(this);
        //InvalidateVisual();

        OnMeasureInvalidated();
    }

    public void Measure(Size availableSize)
    {
        if (float.IsNaN(availableSize.Width) || float.IsNaN(availableSize.Height))
        {
            throw new InvalidOperationException("Cannot call Measure using a size with NaN values.");
        }

        if (!IsMeasureValid || _previousMeasure != availableSize)
        {
            // TODO: Add profile

            Size previousDesiredSize = DesiredSize;
            Size desiredSize = default;

            IsMeasureValid = true;

            try
            {
                _measuring = true;
                desiredSize = MeasureCore(availableSize);
            }
            finally
            {
                _measuring = false;
            }

            if (IsInvalidSize(desiredSize))
            {
                throw new InvalidOperationException("Invalid size returned for Measure.");
            }

            DesiredSize = desiredSize;
            _previousMeasure = availableSize;

            Log.Trace($"Measure requested {DesiredSize}");

            if (DesiredSize != previousDesiredSize)
            {
                Parent?.ChildDesiredSizeChanged(this);
            }
        }
    }

    protected virtual Size MeasureCore(Size availableSize)
    {
        return Size.Empty;
    }

    /// <summary>
    /// Called by InvalidateMeasure
    /// </summary>
    protected virtual void OnMeasureInvalidated()
    {
    }

    /// <summary>
    /// Called when a child element's desired size changes.
    /// </summary>
    /// <param name="child">The child element.</param>
    internal void ChildDesiredSizeChanged(UIElement child)
    {
        if (!_measuring)
        {
            InvalidateMeasure();
        }
    }

    public void Arrange(Rect rect)
    {
        if (IsInvalidRect(rect))
        {
            throw new InvalidOperationException("Invalid Arrange rectangle.");
        }

        if (!IsMeasureValid)
        {
            Measure(_previousMeasure ?? rect.Size);
        }
    }

    /// <summary>
    /// Tests whether any of a <see cref="Size"/>'s properties include negative values,
    /// a NaN or Infinity.
    /// </summary>
    /// <param name="size">The size.</param>
    /// <returns>True if the size is invalid; otherwise false.</returns>
    private static bool IsInvalidSize(Size size)
    {
        return IsNegativeOrNonFinite(size.Width) || IsNegativeOrNonFinite(size.Height);
    }

    /// <summary>
    /// Tests whether any of a <see cref="Rect"/>'s properties include negative values,
    /// a NaN or Infinity.
    /// </summary>
    /// <param name="rect">The rect.</param>
    /// <returns>True if the rect is invalid; otherwise false.</returns>
    private static bool IsInvalidRect(Rect rect)
    {
        return IsNegativeOrNonFinite(rect.Width)
            || IsNegativeOrNonFinite(rect.Height)
            || !float.IsFinite(rect.X)
            || !float.IsFinite(rect.Y);
    }
}
