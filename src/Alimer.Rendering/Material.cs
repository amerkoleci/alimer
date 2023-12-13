// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// Base material class.
/// </summary>
public abstract class Material : DisposableObject
{
    private static int s_nextMaterialId = 1;

    public Material(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        Id = s_nextMaterialId++;
    }

    public GraphicsDevice Device { get; }

    /// <summary>
    /// Gets the unique material id.
    /// </summary>
    public int Id { get; }
}
