// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

public sealed class UnlitMaterial : DisposableObject
{
    private static int s_nextMaterialId = 1;

    public UnlitMaterial(GraphicsDevice device)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;
        Id = s_nextMaterialId++;
    }

    public GraphicsDevice Device { get; }

    /// <summary>
    /// Gets the unique material id.
    /// </summary>
    public int Id { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="UnlitMaterial" /> class.
    /// </summary>
    ~UnlitMaterial() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }
    }
}
