// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SkiaSharp;
using Alimer.Graphics;
using System.Runtime.InteropServices;
using BCnEncoder.Encoder;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Texture"/> asset importer.
/// </summary>
public sealed class TextureImporter : AssetImporter<TextureAsset, TextureMetadata>
{
    public override Task<TextureAsset> Import(TextureMetadata metadata)
    {
        //using Stream stream = await contentManager.FileProvider.OpenStreamAsync(Source, FileMode.Open, FileAccess.Read);
        using Stream stream = File.Open(metadata.FileFullPath, FileMode.Open, FileAccess.Read);

        using (SKBitmap bitmap = SKBitmap.Decode(stream))
        {
            SKBitmap inputImage = bitmap;

            // Resize if necessary
            SKSizeI imageSize = new(bitmap.Width, bitmap.Height);
            SKSizeI targetSize = default;
            if (metadata.IsSizeInPercentage)
            {
                targetSize = new((int)(metadata.Width * imageSize.Width / 100.0f), (int)(metadata.Height * imageSize.Height / 100.0f));
            }

            SKSamplingOptions sampling = new(SKCubicResampler.Mitchell);

            if (targetSize != imageSize)
            {
                inputImage = bitmap.ResizeImage(targetSize, sampling);
            }

            if (metadata.GenerateMipmaps)
            {
                // TODO
                List<SKBitmap> mipLevels = bitmap.GenerateMipmaps(sampling);
                return Task.FromResult(mipLevels[3].ToAsset());
            }

            return Task.FromResult(bitmap.ToAsset());
        }
    }

    private void CompressTexture(TextureAsset texture, TextureMetadata metadata)
    {
        switch (metadata.CompressionFormat)
        {
            case TextureCompressionFormat.BC7:
                BcEncoder encoder = new();
                encoder.OutputOptions.GenerateMipMaps = false; // We've already generated mipmaps
                encoder.OutputOptions.Format = BCnEncoder.Shared.CompressionFormat.Bc7;
                encoder.OutputOptions.Quality = CompressionQuality.Balanced;

                List<byte[]> compressedMips = [];
                int mipWidth = texture.Width;
                int mipHeight = texture.Height;

                for (int i = 0; i < texture.MipLevels; i++)
                {
                    int mipSize = mipWidth * mipHeight * 4;
                    byte[] mipData = texture.Data.Skip(i * mipSize).Take(mipSize).ToArray();
                    byte[] compressedMip = encoder.EncodeToRawBytes(mipData,
                        mipWidth,
                        mipHeight,
                        BCnEncoder.Encoder.PixelFormat.Rgba32,
                        i,
                        out mipWidth,
                        out mipHeight);
                    compressedMips.Add(compressedMip);

                    mipWidth = Math.Max(1, mipWidth / 2);
                    mipHeight = Math.Max(1, mipHeight / 2);
                }

                texture.Data = compressedMips.SelectMany(x => x).ToArray();
                break;

            // Add other compression formats as needed

            default:
                throw new ArgumentException($"Unsupported compression format: {metadata.CompressionFormat}");
        }
    }
}
