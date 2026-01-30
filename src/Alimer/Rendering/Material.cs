// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;

namespace Alimer.Rendering;

public enum MaterialAlphaMode
{
    Opaque,
    Mask,
    Blend
}

/// <summary>
/// Base material class.
/// </summary>
public abstract class Material : Asset
{
    public MaterialAlphaMode AlphaMode { get;set; } = MaterialAlphaMode.Opaque;
    public bool Transparent => AlphaMode == MaterialAlphaMode.Blend;
}
