// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum VertexStepMode
{
    /// <summary>
    /// Vertex data is advanced every vertex.
    /// </summary>
    Vertex = 0,
    /// <summary>
    /// Vertex data is advanced every instance.
    /// </summary>
    Instance = 1
}
