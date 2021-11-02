// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public abstract class Texture : GraphicsResource
    {
        protected Texture(GraphicsDevice device, in TextureDescriptor descriptor)
            : base(device)
        {
            Dimension = descriptor.Dimension;
            Format = descriptor.Format;
            Width = descriptor.Width;
            Height = descriptor.Height;
            Depth = descriptor.Dimension == TextureDimension.Texture3D ? descriptor.DepthOrArraySize : 1;
            ArraySize = descriptor.Dimension != TextureDimension.Texture3D ? descriptor.DepthOrArraySize : 1;
            MipLevels = descriptor.MipLevels;
            SampleCount = descriptor.SampleCount;
            Usage = descriptor.Usage;
        }

        public int CalculateSubresource(int mipSlice, int arraySlice, int planeSlice = 0)
        {
            return mipSlice + arraySlice * MipLevels + planeSlice * MipLevels * ArraySize;
        }

        public TextureDimension Dimension { get; }

        public TextureFormat Format { get; }

        public int Width { get; }
        public int Height { get; }
        public int Depth { get; }
        public int ArraySize { get; }
        public int MipLevels { get; }
        public TextureSampleCount SampleCount { get; }
        public TextureUsage Usage { get; }
    }
}
