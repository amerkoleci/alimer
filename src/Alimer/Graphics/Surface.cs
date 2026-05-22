// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Surface : GraphicsObject
{
    private GraphicsDevice? _device;

    protected Surface(in SurfaceDescriptor descriptor)
        : base(descriptor.Label)
    {
        Source = descriptor.Source;
    }

    public SurfaceSource Source { get; }

    /// <inheritdoc />
    public override GraphicsDevice Device
    {
        get
        {
            if (_device is null)
                throw new InvalidOperationException("Surface is not configured with a graphics device.");

            return _device;
        }
    }

    public int Width { get; protected set; }
    public int Height { get; protected set; }
    public PixelFormat Format { get; protected set; }
    public CompositeAlphaMode AlphaMode { get; protected set; }
    public PresentMode PresentMode { get; protected set; }
    public ColorSpace ColorSpace { get; protected set; } = ColorSpace.SRGB;

    public void Configure(in SurfaceConfiguration configuration)
    {
        _device = configuration.Device;
        Width = configuration.Width;
        Height = configuration.Height;
        Format = configuration.Format;
        AlphaMode = configuration.AlphaMode;
        PresentMode = configuration.PresentMode;
        ConfigureCore();
    }

    public void Unconfigure()
    {
        _device = null;
        Width = 0;
        Height = 0;
        Format = PixelFormat.Undefined;
        AlphaMode = CompositeAlphaMode.Auto;
        PresentMode = PresentMode.Fifo;
        UnconfigureCore();
    }

    public void Resize(int newWidth, int newHeight)
    {
        if (Width == newWidth && Height == newHeight)
            return;

        ResizeCore(newWidth, newHeight);
        Width = newWidth;
        Height = newHeight;
    }

    protected abstract void ConfigureCore();
    protected abstract void UnconfigureCore();
    protected abstract void ResizeCore(int newWidth, int newHeight);

    /// <summary>
    /// Acquires the next available texture for rendering or processing operations.
    /// </summary>
    /// <returns>A <see cref="Texture"/> object representing the next available texture, or <see langword="null"/> if no texture
    /// is currently available.</returns>
    public abstract Texture? AcquireNextTexture();
}
