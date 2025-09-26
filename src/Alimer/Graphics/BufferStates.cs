// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Enum indicating <see cref="GraphicsBuffer"/> state.
/// </summary>
[Flags]
public enum BufferStates : uint
{
    Undefined = 0,
    CopyDest = 1 << 0,
    CopySource = 1 << 1,
    ShaderResource = 1 << 2,
    UnorderedAccess = 1 << 3,
    VertexBuffer = 1 << 4,
    IndexBuffer = 1 << 5,
    ConstantBuffer = 1 << 6,
    Predication = 1 << 7,
}
