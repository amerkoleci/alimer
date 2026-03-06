// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Assets;

/// <summary>
/// Defines an attribute to be used on asset importer classes to specify the supported file extensions for that importer.
/// </summary>
[AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
public class AssetImporterAttribute : Attribute
{
    public AssetImporterAttribute(params string[] fileExtensions)
    {
        foreach (string text in fileExtensions)
        {
            ArgumentException.ThrowIfNullOrEmpty(text, nameof(fileExtensions));
        }

        SupportedFileExtensions = Array.AsReadOnly(fileExtensions);
    }

    public ReadOnlyCollection<string> SupportedFileExtensions { get; }
}
