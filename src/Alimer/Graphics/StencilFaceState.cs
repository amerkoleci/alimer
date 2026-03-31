// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public record struct StencilFaceState
{
    public StencilOperation FailOperation;
    public StencilOperation DepthFailOperation;
    public StencilOperation PassOperation;
    public CompareFunction CompareFunction;

    /// <summary>
    /// A built-in description with default values.
    /// </summary>
    public static StencilFaceState Default => new();

    /// <summary>
    /// Initializes a new instance of the <see cref="DepthStencilState"/> struct with default values.
    /// </summary>
    public StencilFaceState()
    {
        FailOperation = StencilOperation.Keep;
        DepthFailOperation = StencilOperation.Keep;
        PassOperation = StencilOperation.Keep;
        CompareFunction = CompareFunction.Always;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="StencilFaceState"/> struct.
    /// </summary>
    /// <param name="failOperation">The operation performed on samples that fail the stencil test.</param>
    /// <param name="depthFailOperation">The operation performed on samples that pass the stencil test but fail the depth test.</param>
    /// <param name="passOperation">The operation performed on samples that pass the stencil test.</param>
    /// <param name="compareFunction">The comparison operator used in the stencil test.</param>
    public StencilFaceState(
        StencilOperation failOperation,
        StencilOperation depthFailOperation,
        StencilOperation passOperation,
        CompareFunction compareFunction)
    {
        FailOperation = failOperation;
        DepthFailOperation = depthFailOperation;
        PassOperation = passOperation;
        CompareFunction = compareFunction;
    }
}
