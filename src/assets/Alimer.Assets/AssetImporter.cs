// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public abstract class AssetImporter<T> : IAssetImporter where T : Asset
{
    public abstract Task<T> Import(string source, IServiceRegistry services);

    Task<Asset> IAssetImporter.Import(string source, IServiceRegistry services) => Import(source, services).ContinueWith(t => (Asset)t.Result);
}
