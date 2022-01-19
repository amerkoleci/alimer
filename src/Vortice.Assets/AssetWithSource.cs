// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Assets;

public abstract class AssetWithSource : Asset
{
    public string Source { get; set; } = string.Empty;
}
