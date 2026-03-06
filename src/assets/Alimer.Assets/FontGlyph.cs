// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Numerics;

namespace Alimer.Assets;

public struct FontGlyph
{
    /// <summary>
    /// Unicode codepoint.
    /// </summary>
    public int Character;

    /// <summary>
    /// Glyph image data (may only use a portion of a larger bitmap).
    /// </summary>
    public RectI Subrect;

    /// <summary>
    /// Glyph X and Y offset from origin.
    /// </summary>
    public Vector2 Offset;

    /// <summary>
    /// Horizontal advance.
    /// </summary>
    public float XAdvance;
}
