// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics.VGPU;
using static Alimer.Graphics.VGPU.VGPU;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics;

public sealed class SwapChain : GraphicsObject
{
    internal VGPUSwapChain Handle { get; }

    public SwapChainSurface Surface { get; }

    public PixelFormat ColorFormat { get; private set; }
    public PresentMode PresentMode { get; }
    public bool IsFullscreen { get; private set; }

    public bool AutoResizeDrawable { get; set; } = true;
    public Size Size
    {
        get
        {
            vgpuSwapChainGetSize(Handle, out uint width, out uint height);
            return new((int)width, (int)height);
        }
    }

    public unsafe SwapChain(GraphicsDevice device, SwapChainSurface surface, in SwapChainDescription description)
        : base(device, true, description.Label)
    {
        Surface = surface;
        ColorFormat = description.Format;
        PresentMode = description.PresentMode;
        IsFullscreen = description.IsFullscreen;

        fixed (sbyte* pLabel = description.Label.GetUtf8Span())
        {
            VGPUSwapChainDesc nativeDesc = new()
            {
                label = pLabel,
                width = (uint)description.Width,
                height = (uint)description.Height,
                format = (VGPUTextureFormat)description.Format, // VGPUTextureFormat_BGRA8UnormSrgb;
                presentMode = (VGPUPresentMode)description.PresentMode,
                isFullscreen = IsFullscreen
            };
            Handle = vgpuCreateSwapChain(device.Handle, ((Win32SwapChainSurface)surface).Hwnd, &nativeDesc);
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="SwapChain" /> class.
    /// </summary>
    ~SwapChain() => Dispose(disposing: false);

    protected override void Destroy()
    {
        _ = vgpuSwapChainRelease(Handle);
    }

    //protected abstract void ResizeBackBuffer(int width, int height);
}
