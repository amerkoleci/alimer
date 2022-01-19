// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Assets;

public abstract class AssetImporter<T> : IAssetImporter where T : Asset
{
    public abstract Task<T> Import(string source, IServiceProvider services);

    Task<Asset> IAssetImporter.Import(string source, IServiceProvider services) => Import(source, services).ContinueWith(t => (Asset)t.Result);
}
