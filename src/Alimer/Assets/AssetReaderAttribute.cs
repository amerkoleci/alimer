// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Initializes a new instance of the <see cref="AssetReaderAttribute" /> class.
/// </summary>
/// <param name="assetReaderType">Type of the asset reader.</param>
[AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct, Inherited = false, AllowMultiple = false)]
public class AssetReaderAttribute(Type assetReaderType) : Attribute
{
    /// <summary>
    /// Gets the type of the asset reader.
    /// </summary>
    public Type AssetReaderType { get; } = assetReaderType;
}
