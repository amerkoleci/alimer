// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets.Graphics;
using Alimer.Assets.Writers;

namespace Alimer.Assets.Compiler;

public class AssetCompilerContext : IDisposable
{
    private const string MetadataFileExtension = ".ameta";
    private readonly List<string> _allIncludeDirectories = [];
    private readonly HashSet<string> _includedFiles = [];
    private static readonly string[] IgnoredFiles = ["CMakeLists.txt"];
    private static readonly string[] IgnoredFileExtensions = [".md", ".hlsli"];

    private readonly Dictionary<Type, IAssetImporter> _importers = [];
    private readonly Dictionary<Type, IAssetTypeWriter> _writers = [];

    public AssetCompilerContext()
    {
        // Importers
        //RegisterImporter<ShaderImporter>();
        RegisterImporter<TextureImporter>();
        //RegisterImporter<FontImporter>();

        // Writers
        RegisterWriter<TextureWriter>();
        //RegisterWriter<FontWriter>();
    }


    public void Dispose()
    {
    }

    public void RegisterImporter<T>() where T : IAssetImporter, new()
    {
        T assetImporter = new();
        _importers.Add(assetImporter.AssetMetadataType, assetImporter);
    }

    public void RegisterWriter<T>() where T : IAssetTypeWriter, new()
    {
        T assetWriter = new();
        _writers.Add(assetWriter.AssetType, assetWriter);
    }


    public void Reset()
    {
        _allIncludeDirectories.Clear();
        _includedFiles.Clear();
    }

    public async Task<bool> Run(string inputDirectory, string outputDirectory, TextWriter output)
    {
        // We have always the search path of the current directory
        _allIncludeDirectories.Clear();
        //_allIncludeDirectories.AddRange(_app.IncludeDirectories);
        //_allIncludeDirectories.Add(_app.CurrentDirectory);

        IEnumerable<string> files = EnumerateFiles(inputDirectory, $"*{MetadataFileExtension}")
            .Where(f => !IsIgnored(f));

        foreach (string path in files)
        {
            string fileName = Path.GetFileName(path);
            string directory = Path.GetDirectoryName(path)!;
            string relativePath = Path.GetRelativePath(inputDirectory, path);

            AssetMetadata? metadata = AssetMetadata.FromFile(path);
            if (metadata is null)
                continue;

            string assetFileName = Path.GetFileNameWithoutExtension(path);
            metadata.FileFullPath = Path.Combine(directory, assetFileName);
            metadata.FileRelativePath = Path.GetRelativePath(inputDirectory, metadata.FileFullPath);

            //string json = metadata.ToJson();
            Asset? asset = await Import(metadata) ?? throw new NullReferenceException("Asset is null");
            if (_writers.TryGetValue(asset.GetType(), out IAssetTypeWriter? assetWriter))
            {
                string outputFileName = Path.ChangeExtension(Path.GetFileNameWithoutExtension(assetFileName), assetWriter.FileExtension);
                string outputFilePath = Path.Combine(outputDirectory, outputFileName);

                using (MemoryStream ms = new())
                {
                    using (AssetWriter writer = new(ms))
                    {
                        assetWriter.Write(writer, asset);
                    }

                    byte[] assetData = ms.ToArray();
                    File.WriteAllBytes(outputFilePath, assetData);
                }
            }
            else
            {
                throw new InvalidOperationException($"No writer found for {asset.GetType()}");
            }
        }

        return true;
    }

    private async Task<Asset?> Import(AssetMetadata metadata)
    {
        if (_importers.TryGetValue(metadata.GetType(), out IAssetImporter? importer))
        {
            Asset asset = await importer.Import(metadata);
            return asset;
        }

        return default;
    }

    private static bool IsIgnored(string file)
        => IgnoredFiles.Any(item => item == Path.GetFileName(file)) || IgnoredFileExtensions.Any(ext => Path.GetExtension(file).Equals(ext, StringComparison.OrdinalIgnoreCase));

    private static IEnumerable<string> EnumerateFiles(string basePath, string pattern)
        => Directory.EnumerateFiles(basePath, pattern, SearchOption.AllDirectories);
}
