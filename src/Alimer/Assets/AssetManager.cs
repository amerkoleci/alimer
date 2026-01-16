// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public class AssetManager : DisposableObject, IAssetManager
{
    public AssetManager(string rootDirectory)
    {
        RootDirectory = rootDirectory;
    }

    public string RootDirectory { get; } 

    protected override void Dispose(bool disposing)
    {

    }
}
