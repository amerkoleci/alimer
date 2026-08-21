// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Native.AlimerGPUApi;

namespace Alimer.Graphics.Native;

internal unsafe class NativeSurface : Surface
{
    public NativeSurface(NativeGraphicsManager manager, in SurfaceDescriptor descriptor)
        : base(in descriptor)
    {
        switch (descriptor.Source)
        {
            case Win32SwapChainSurface win32Surface:
                SourceHandle = agpuSurfaceSourceCreateFromWin32(win32Surface.Hwnd);
                break;
            case AndroidSwapChainSurface androidSurface:
                SourceHandle = agpuSurfaceSourceCreateFromAndroid(androidSurface.Window);
                break;
            case MetalLayerChainSurface metalLayerSurface:
                SourceHandle = agpuSurfaceSourceCreateFromMetalLayer(metalLayerSurface.Layer);
                break;
            case WaylandSwapChainSurface waylandSurface:
                SourceHandle = agpuSurfaceSourceCreateFromWaylandSurface(waylandSurface.Display, waylandSurface.Surface);
                break;
            case XlibSwapChainSurface xlibSurface:
                SourceHandle = agpuSurfaceSourceCreateFromXlibWindow(xlibSurface.Display, xlibSurface.Window);
                break;
            default:
                throw new NotSupportedException($"Unsupported surface source type: {descriptor.Source}");
        }

        Handle = agpuFactoryCreateSurface(manager.Handle, SourceHandle);
    }

    public GPUSurfaceSource SourceHandle { get; }
    public GPUSurface Handle { get; }
    protected internal override void Destroy()
    {
        _ = agpuSurfaceRelease(Handle);
        agpuSurfaceSourceDestroy(SourceHandle);
    }

    public override Texture? AcquireNextTexture() => throw new NotImplementedException();
    protected override void ConfigureCore()
    {
        GPUSurfaceConfig config = new()
        {
            device = ((NativeGraphicsDevice)Device).Handle,
            width = (uint)Width,
            height = (uint)Height,
            format = Format,
            presentMode = PresentMode,
        };
        if (!agpuSurfaceConfigure(Handle, &config))
        {
            throw new GraphicsException("Failed to configure surface.");
        }
    }

    protected override void ResizeCore(int newWidth, int newHeight) => throw new NotImplementedException();
    protected override void UnconfigureCore() => throw new NotImplementedException();
}
