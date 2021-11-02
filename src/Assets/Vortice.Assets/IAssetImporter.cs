// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Threading.Tasks;

namespace Vortice.Assets
{
    public interface IAssetImporter
    {
        Task<Asset> Import(string source);
    }
}
