// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines dimension of <see cref="TextureView"/>
/// </summary>
public enum TextureViewDimension
{
    /// <summary>
    /// Undefined, inherits from <see cref="Texture"/>
    /// </summary>
    Undefined,
    /// <summary>
    /// 1D texture view.
    /// </summary>
	View1D,
    /// <summary>
    /// 2D texture view.
    /// </summary>
    View2D,
    /// <summary>
    /// 3D texture view.
    /// </summary>
    View3D,
    /// <summary>
    /// Cube texture view.
    /// </summary>
    ViewCube,
    /// <summary>
    /// 1D texture array view.
    /// </summary>
    View1DArray,
    /// <summary>
    /// 2D texture array view.
    /// </summary>
    Texture2DArray,
    /// <summary>
    /// Cube texture array view.
    /// </summary>
    ViewCubeArray,
}
