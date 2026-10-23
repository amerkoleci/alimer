// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;

namespace Alimer.Input;

public sealed partial class Cursor
{
    public CursorType CursorType { get; }

    internal partial Cursor(CursorType type);

    public partial Cursor(ReadOnlySpan<ColorRgba> pixels, in SizeI size, in PointI hotspot);
}
