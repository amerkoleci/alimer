// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct StencilFaceState
{
    public StencilFaceState()
    {
        FailOperation = StencilOperation.Keep;
        DepthFailOperation = StencilOperation.Keep;
        PassOperation = StencilOperation.Keep;
        CompareFunction = CompareFunction.Always;
    }

    public StencilOperation FailOperation { get; init; }
    public StencilOperation DepthFailOperation { get; init; }
    public StencilOperation PassOperation { get; init; }
    public CompareFunction CompareFunction { get; init; }
}
