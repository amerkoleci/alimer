// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Rendering;

namespace Alimer.Engine;

public sealed partial class MeshComponent : RenderableComponent
{
    public MeshComponent()
    {
    }

    public MeshComponent(Mesh mesh)
    {
        Mesh = mesh;
    }

    public Mesh? Mesh { get; set; }
}
