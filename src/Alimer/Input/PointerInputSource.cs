// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// Represents an abstract input source for pointer events, providing a base for handling pointer interactions such as
/// entering, exiting, moving, pressing, and releasing.
/// </summary>
public abstract class PointerInputSource : IInputSource
{
    public event EventHandler<PointerEventArgs>? PointerEntered;
    public event EventHandler<PointerEventArgs>? PointerExited;
    public event EventHandler<PointerEventArgs>? PointerMoved;
    public event EventHandler<PointerEventArgs>? PointerPressed;
    public event EventHandler<PointerEventArgs>? PointerReleased;
    public event EventHandler<PointerEventArgs>? PointerWheelChanged;

    /// <summary>
    /// Gets the current position in pixels.
    /// </summary>
	public abstract Vector2 Position { get; }

    /// <summary>
    /// Gets the movement since last frame.
    /// </summary>
	public abstract Vector2 Delta { get; }

    /// <summary>
    /// Gets the horizontal and vertical scroll amount this frame.
    /// </summary>
	public abstract Vector2 Scroll { get; }

    /// <summary>
    /// Gets or sets whether the mouse cursor is visible.
    /// </summary>
    public abstract bool IsCursorVisible { get; set; }

    /// <summary>
    /// Gets or sets the current cursor type.
    /// </summary>
    public abstract Cursor Cursor { get; set; }

    public virtual void Scan()
    {
    }

    public virtual void Update()
    {
    }

    /// <summary>
    /// Returns true if the button is currently held down.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
	public abstract bool IsButtonDown(MouseButton button);

    /// <summary>
    /// Returns true if the button was just pressed this frame.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    public abstract bool IsButtonPressed(MouseButton button);

    /// <summary>
    /// Returns true if the button was just released this frame.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    public abstract bool IsButtonReleased(MouseButton button);

    protected virtual void OnPointerEntered(in PointerEventArgs e)
    {
        PointerEntered?.Invoke(this, e);
    }

    protected virtual void OnPointerExited(in PointerEventArgs e)
    {
        PointerExited?.Invoke(this, e);
    }

    protected virtual void OnPointerMoved(in PointerEventArgs e)
    {
        PointerMoved?.Invoke(this, e);
    }

    protected virtual void OnPointerPressed(in PointerEventArgs e)
    {
        PointerPressed?.Invoke(this, e);
    }

    protected virtual void OnPointerReleased(in PointerEventArgs e)
    {
        PointerReleased?.Invoke(this, e);
    }

    protected virtual void OnPointerWheelChanged(in PointerEventArgs e)
    {
        PointerWheelChanged?.Invoke(this, e);
    }
}
