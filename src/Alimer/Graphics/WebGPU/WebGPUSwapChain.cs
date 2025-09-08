// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUSwapChain : SwapChain
{
    private readonly WebGPUGraphicsDevice _device;
    private readonly WGPUSurface _surface;
    private WGPUTexture _acquiredTexture = default;
    private readonly Dictionary<WGPUTexture, WebGPUTexture> _backendTextures = [];

    public WebGPUSwapChain(WebGPUGraphicsDevice device, ISwapChainSurface surfaceSource, in SwapChainDescription descriptor)
        : base(surfaceSource, descriptor)
    {
        _device = device;
        // Create surface first.
        _surface = CreateWebGPUSurface();

        AfterReset();

        WGPUSurface CreateWebGPUSurface()
        {
            switch (surfaceSource.Kind)
            {
                case SwapChainSurfaceType.Win32:
                    {
                        WGPUSurfaceDescriptorFromWindowsHWND chain = new()
                        {
                            hinstance = surfaceSource.ContextHandle,
                            hwnd = surfaceSource.Handle,
                            chain = new WGPUChainedStruct()
                            {
                                sType = WGPUSType.SurfaceDescriptorFromWindowsHWND
                            }
                        };
                        WGPUSurfaceDescriptor descriptor = new()
                        {
                            nextInChain = (WGPUChainedStruct*)&chain
                        };
                        return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }

                case SwapChainSurfaceType.Android:
                    {
                        WGPUSurfaceDescriptorFromAndroidNativeWindow chain = new()
                        {
                            window = surfaceSource.Handle,
                            chain = new WGPUChainedStruct()
                            {
                                sType = WGPUSType.SurfaceDescriptorFromAndroidNativeWindow
                            }
                        };
                        WGPUSurfaceDescriptor descriptor = new()
                        {
                            nextInChain = (WGPUChainedStruct*)&chain
                        };
                        return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }

                case SwapChainSurfaceType.Wayland:
                    {
                        WGPUSurfaceDescriptorFromWaylandSurface chain = new()
                        {
                            display = surfaceSource.ContextHandle,
                            surface = surfaceSource.Handle,
                            chain = new WGPUChainedStruct()
                            {
                                sType = WGPUSType.SurfaceDescriptorFromWaylandSurface
                            }
                        };
                        WGPUSurfaceDescriptor descriptor = new()
                        {
                            nextInChain = (WGPUChainedStruct*)&chain
                        };
                        return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }

                case SwapChainSurfaceType.Xcb:
                    {
                        WGPUSurfaceDescriptorFromXcbWindow chain = new()
                        {
                            connection = surfaceSource.ContextHandle,
                            window = (uint)(nuint)surfaceSource.Handle,
                            chain = new WGPUChainedStruct()
                            {
                                sType = WGPUSType.SurfaceDescriptorFromXcbWindow
                            }
                        };
                        WGPUSurfaceDescriptor descriptor = new()
                        {
                            nextInChain = (WGPUChainedStruct*)&chain
                        };
                        return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }

                case SwapChainSurfaceType.Xlib:
                    {
                        WGPUSurfaceDescriptorFromXlibWindow chain = new()
                        {
                            display = surfaceSource.ContextHandle,
                            window = (uint)(nuint)surfaceSource.Handle,
                            chain = new WGPUChainedStruct()
                            {
                                sType = WGPUSType.SurfaceDescriptorFromXlibWindow
                            }
                        };
                        WGPUSurfaceDescriptor descriptor = new()
                        {
                            nextInChain = (WGPUChainedStruct*)&chain
                        };
                        return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }

                case SwapChainSurfaceType.MetalLayer:
                    {
                        //NSWindow ns_window = new(glfwGetCocoaWindow(_window));
                        //CAMetalLayer metal_layer = CAMetalLayer.New();
                        //ns_window.contentView.wantsLayer = true;
                        //ns_window.contentView.layer = metal_layer.Handle;
                        //
                        //WGPUSurfaceDescriptorFromMetalLayer chain = new()
                        //{
                        //    layer = metal_layer.Handle,
                        //    chain = new WGPUChainedStruct()
                        //    {
                        //        sType = WGPUSType.SurfaceDescriptorFromMetalLayer
                        //    }
                        //};
                        //WGPUSurfaceDescriptor descriptor = new()
                        //{
                        //    nextInChain = (WGPUChainedStruct*)&chain
                        //};
                        //return wgpuInstanceCreateSurface(device.Instance, &descriptor);
                    }
                    throw new GraphicsException($"WebGPU: MetalLayer is not implemented!");

                default:
                    throw new GraphicsException($"WebGPU: Invalid kind for surface: {surfaceSource.Kind}");
            }
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public WGPUTextureFormat SwapChainFormat { get; private set; }
    public WGPUSurface Handle => _surface;

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUSwapChain" /> class.
    /// </summary>
    ~WebGPUSwapChain() => Dispose(disposing: false);

    private void AfterReset()
    {
        // WGPUTextureFormat_BGRA8UnormSrgb on desktop, WGPUTextureFormat_BGRA8Unorm on mobile
        SwapChainFormat = wgpuSurfaceGetPreferredFormat(_surface, _device.Adapter);
        Debug.Assert(SwapChainFormat != WGPUTextureFormat.Undefined);

        WGPUTextureFormat viewFormat = SwapChainFormat;
        WGPUSurfaceConfiguration surfaceConfiguration = new()
        {
            nextInChain = null,
            device = _device.Handle,
            format = SwapChainFormat,
            usage = WGPUTextureUsage.RenderAttachment,
            viewFormatCount = 1,
            viewFormats = &viewFormat,
            alphaMode = WGPUCompositeAlphaMode.Auto,
            width = (uint)DrawableSize.Width,
            height = (uint)DrawableSize.Height,
            presentMode = PresentMode.ToWebGPU(),
        };
        wgpuSurfaceConfigure(_surface, &surfaceConfiguration);
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {

        }

        base.Dispose(disposing);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuSurfaceRelease(_surface);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        //_handle.Get()->SetDebugName(newLabel);
    }

    protected override void ResizeBackBuffer()
    {

    }

    public override Texture? GetCurrentTexture()
    {
        WGPUSurfaceTexture surfaceTexture = default;
        wgpuSurfaceGetCurrentTexture(_surface, &surfaceTexture);

        // Getting the texture may fail, in particular if the window has been resized
        // and thus the target surface changed.
        if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus.Timeout)
        {
            Log.Error("Cannot acquire next swap chain texture");
            return default;
        }

        if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus.Outdated)
        {
            Log.Warn("Surface texture is outdated, reconfigure the surface!");
            return default;
        }

        _acquiredTexture = surfaceTexture.texture;
        if (!_backendTextures.TryGetValue(surfaceTexture.texture, out WebGPUTexture? texture))
        {
            texture = new WebGPUTexture(_device, surfaceTexture.texture, TextureDescriptor.Texture2D(PixelFormat.BGRA8UnormSrgb, (uint)DrawableSize.Width, (uint)DrawableSize.Height));
            _backendTextures.Add(surfaceTexture.texture, texture);
        }

        return texture;
    }

    public void ReleaseCurrentTexture()
    {
        if (_acquiredTexture.IsNull)
            return;

        //wgpuTextureRelease(_acquiredTexture);
        _acquiredTexture = WGPUTexture.Null;
    }
}
