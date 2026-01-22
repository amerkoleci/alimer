// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

partial class Mesh 
{
    public static Mesh CreateCube(GraphicsDevice device, float size)
    {
        return CreateBox(device, new Vector3(size));
    }

    public static Mesh CreateBox(GraphicsDevice device, in Vector3 size)
    {
        // TODO
        Mesh mesh = new(device);
        return mesh;
    }
}
