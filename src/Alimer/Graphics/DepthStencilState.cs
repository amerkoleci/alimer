// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public record struct DepthStencilState
{
    public bool DepthWriteEnabled;
    public CompareFunction DepthCompare;

    public byte StencilReadMask;
    public byte StencilWriteMask;

    public StencilFaceState StencilFront;
    public StencilFaceState StencilBack;

    public bool DepthBoundsTestEnable;

    public static DepthStencilState DepthNone => new(false, CompareFunction.Always);

    public static DepthStencilState DepthDefault => new(true, CompareFunction.LessEqual);

    public static DepthStencilState DepthRead => new(false, CompareFunction.LessEqual);

    public static DepthStencilState DepthReverseZ => new(true, CompareFunction.GreaterEqual);

    public static DepthStencilState DepthReadReverseZ => new(false, CompareFunction.GreaterEqual);

    /// <summary>
    /// Initializes a new instance of the <see cref="DepthStencilState"/> struct with default values.
    /// </summary>
    public DepthStencilState()
    {
        DepthWriteEnabled = true;
        DepthCompare = CompareFunction.LessEqual;

        StencilReadMask = 0xFF;
        StencilWriteMask = 0xFF;
        StencilFront = StencilFaceState.Default;
        StencilBack = StencilFaceState.Default;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="DepthStencilState"/> struct. This describes a depth-stencil state with no stencil testing enabled.
    /// </summary>
    /// <param name="depthWriteEnabled">Controls whether new depth values are written to the depth buffer.</param>
    /// <param name="depthCompare">The <see cref="CompareFunction"/> used when considering new depth values.</param>
    public DepthStencilState(bool depthWriteEnabled, CompareFunction depthCompare)
    {
        DepthWriteEnabled = depthWriteEnabled;
        DepthCompare = depthCompare;

        StencilReadMask = 0xFF;
        StencilWriteMask = 0xFF;
        StencilFront = StencilFaceState.Default;
        StencilBack = StencilFaceState.Default;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="DepthStencilState"/> struct. 
    /// </summary>
    /// <param name="depthWriteEnabled">Controls whether new depth values are written to the depth buffer.</param>
    /// <param name="depthCompare">The <see cref="CompareFunction"/> used when considering new depth values.</param>
    /// <param name="stencilFront">Controls how stencil tests are handled for pixels whose surface faces towards the camera.</param>
    /// <param name="stencilBack">Controls how stencil tests are handled for pixels whose surface faces away from the camera.</param>
    /// <param name="stencilReadMask">Controls the portion of the stencil buffer used for reading.</param>
    /// <param name="stencilWriteMask">Controls the portion of the stencil buffer used for writing.</param>
    public DepthStencilState(
        bool depthWriteEnabled,
        CompareFunction depthCompare,
        StencilFaceState stencilFront,
        StencilFaceState stencilBack,
        byte stencilReadMask,
        byte stencilWriteMask)
    {
        DepthWriteEnabled = depthWriteEnabled;
        DepthCompare = depthCompare;

        StencilFront = stencilFront;
        StencilBack = stencilBack;
        StencilReadMask = stencilReadMask;
        StencilWriteMask = stencilWriteMask;
    }
}
