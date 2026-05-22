// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class TextureView 
{
    private string? _label;

    protected TextureView(Texture texture, in TextureViewDescriptor descriptor)
    {
        ArgumentNullException.ThrowIfNull(texture, nameof(texture));

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
    /// Gets the bindless read index of the view.
    /// </summary>
    public abstract int BindlessReadIndex { get; }

    /// <summary>
    /// Gets the bindless read-write index of the view.
    /// </summary>
    public abstract int BindlessReadWriteIndex { get; }

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

    public uint Width => Texture.GetLevelWidth(BaseMipLevel);
    public uint Height => Texture.GetLevelHeight(BaseMipLevel);

    protected virtual void OnLabelChanged(string? newLabel)
    {
    }

    internal abstract void Destroy();

    /// <summary>
    /// Get a native handle for this object.
    /// The type of the handle is determined by the <see cref="GraphicsNativeHandleType"/> parameter.
    /// The returned handle is platform and API specific, and may not be valid across different platforms or graphics APIs.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    public virtual GraphicsNativeHandle GetNativeHandle(GraphicsNativeHandleType type) => GraphicsNativeHandle.InvalidHandle;
}
