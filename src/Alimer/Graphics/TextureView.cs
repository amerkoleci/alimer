// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract class TextureView
{
    private string? _label;

    protected TextureView(Texture texture, in TextureViewDescriptor descriptor)
    {
        Guard.IsNotNull(texture, nameof(texture));

        Texture = texture;
        Dimension = descriptor.Dimension;
        Format = descriptor.Format;
        BaseMipLevel = descriptor.BaseMipLevel;
        MipLevelCount = descriptor.MipLevelCount;
        BaseArrayLayer = descriptor.BaseArrayLayer;
        ArrayLayerCount = descriptor.ArrayLayerCount;
        Swizzle = descriptor.Swizzle;
        Aspect = descriptor.Aspect;
        _label = descriptor.Label;
    }

    public Texture Texture { get; }
    public TextureViewDimension Dimension { get; }
    public PixelFormat Format { get; }
    public uint BaseMipLevel { get; }
    public uint MipLevelCount { get; }
    public uint BaseArrayLayer { get; }
    public uint ArrayLayerCount { get; }
    public TextureSwizzleChannels Swizzle { get; }
    public TextureAspect Aspect { get; }

    /// <summary>
    /// Gets or sets the label that identifies this object.
    /// </summary>
    public string? Label
    {
        get => _label;
        set
        {
            if (_label == value)
                return;

            _label = value;
            OnLabelChanged(value);
        }
    }

    public uint Width => Texture.GetWidth(BaseMipLevel);
    public uint Height => Texture.GetHeight(BaseMipLevel);

    protected virtual void OnLabelChanged(string? newLabel)
    {
    }

    internal abstract void Destroy();
}
