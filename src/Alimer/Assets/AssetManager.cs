// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public class AssetManager : DisposableObject, IAssetManager
{
    private readonly IServiceRegistry _services;

    public AssetManager(IServiceRegistry services)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        _services = services;
    }

    protected override void Dispose(bool disposing)
    {

    }
}
