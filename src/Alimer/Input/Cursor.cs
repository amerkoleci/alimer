// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public abstract class Cursor
{
    public CursorType CursorType { get; }

    protected Cursor()
    {
        CursorType = CursorType.Custom;
    }

    protected Cursor(CursorType cursorType)
    {
        CursorType = cursorType;
    }
}
