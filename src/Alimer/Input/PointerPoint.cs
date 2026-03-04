// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public readonly struct PointerPoint
{
    public required bool IsInContact { get; init; }
    public required uint PointerId { get; init; }
    public required Vector2 Position { get; init; }
    public MouseButton Button { get; init; }
}
