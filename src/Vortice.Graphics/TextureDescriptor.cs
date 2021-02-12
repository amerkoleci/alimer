// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Vortice.Graphics
{
    public readonly struct TextureDescriptor
    {
        public TextureDescriptor(int width, int height, TextureFormat format = TextureFormat.RGBA8Unorm, int arrayLayers = 1, int mipmapLevelCount = 0, TextureUsage usage = TextureUsage.Sampled)
        {
            Dimension = TextureDimension.Texture2D;
            Usage = usage;
            Width = width;
            Height = height;
            Depth = 1;
            ArrayLayers = arrayLayers;
            Format = format;
            MipmapLevelCount = mipmapLevelCount;
            SampleCount = 1;
        }

        public TextureDimension Dimension { get; }

        public TextureUsage Usage { get; }

        public int Width { get; }
        public int Height { get; }
        public int Depth { get; }
        public int ArrayLayers { get; }
        public TextureFormat Format { get; }
        public int MipmapLevelCount { get; }
        public int SampleCount { get; }
    }
}
