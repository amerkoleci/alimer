// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public static partial class Cursors
{
    private static readonly int s_cursorTypeCount = (int)CursorType.Custom + 1;
    private static readonly Cursor[] s_stockCursors = new Cursor[s_cursorTypeCount];

    public static Cursor Arrow => EnsureCursor(CursorType.Arrow);
    public static Cursor Cross => EnsureCursor(CursorType.Cross);
    public static Cursor Hand => EnsureCursor(CursorType.Hand);
    public static Cursor Wait => EnsureCursor(CursorType.Wait);
    public static Cursor IBeam => EnsureCursor(CursorType.IBeam);

    /// <summary>
    /// Progress cursor (busy but interactive).
    /// </summary>
	public static Cursor Progress => EnsureCursor(CursorType.Progress);
    /// Resize cursor (northwest-southeast).
    public static Cursor ResizeNWSE => EnsureCursor(CursorType.ResizeNWSE);
    /// Resize cursor (northeast-southwest).
    public static Cursor ResizeNESW => EnsureCursor(CursorType.ResizeNESW);
    /// Resize cursor (east-west).
    public static Cursor ResizeEW => EnsureCursor(CursorType.ResizeEW);
    /// Resize cursor (north-south).
    public static Cursor ResizeNS => EnsureCursor(CursorType.ResizeNS);
    /// Move cursor (four arrows).
    public static Cursor Move => EnsureCursor(CursorType.Move);
    /// Not allowed cursor (slashed circle).
    public static Cursor NotAllowed => EnsureCursor(CursorType.NotAllowed);
    /// Pointer/hand cursor (for links/buttons).
    public static Cursor Pointer => EnsureCursor(CursorType.Pointer);
    /// Resize cursor for top-left corner.
    public static Cursor ResizeNW => EnsureCursor(CursorType.ResizeNW);
    /// Resize cursor for top edge.
    public static Cursor ResizeN => EnsureCursor(CursorType.ResizeN);
    /// Resize cursor for top-right corner.
    public static Cursor ResizeNE => EnsureCursor(CursorType.ResizeNE);
    /// Resize cursor for right edge.
    public static Cursor ResizeE => EnsureCursor(CursorType.ResizeE);
    /// Resize cursor for bottom-right corner.
    public static Cursor ResizeSE => EnsureCursor(CursorType.ResizeSE);
    /// Resize cursor for bottom edge.
    public static Cursor ResizeS => EnsureCursor(CursorType.ResizeS);
    /// Resize cursor for bottom-left corner.
    public static Cursor ResizeSW => EnsureCursor(CursorType.ResizeSW);
    /// Resize cursor for left edge.
    public static Cursor ResizeW => EnsureCursor(CursorType.ResizeW);

    internal static Cursor EnsureCursor(CursorType cursorType)
    {
        if (s_stockCursors[(int)cursorType] == null)
        {
            s_stockCursors[(int)cursorType] = CreateCursor(cursorType);
        }

        return s_stockCursors[(int)cursorType];
    }

    internal static void Shutdown()
    {
        for (int i = 0; i < s_cursorTypeCount; i++)
        {
            if (s_stockCursors[i] is not null
                && s_stockCursors[i] is IDisposable disposable)
            {
                disposable.Dispose();
            }
        }
    }

    private static partial Cursor CreateCursor(CursorType cursorType);
}
