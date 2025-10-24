// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeSwapChain : SwapChain
{
    private readonly NativeGraphicsDevice _device;
    private readonly GPUSurfaceHandle _surface;

    public NativeSwapChain(NativeGraphicsDevice device, ISwapChainSurface surfaceSource, in SwapChainDescription descriptor)
        : base(surfaceSource, descriptor)
    {
        _device = device;

        switch (surfaceSource.Kind)
        {
            case SwapChainSurfaceType.Win32:
                _surface = agpuSurfaceHandleCreateFromWin32(surfaceSource.ContextHandle, surfaceSource.Handle);
                break;
            default:
                _surface = GPUSurfaceHandle.Null;
                break;
        }
    }

    public override GraphicsDevice Device => _device;

    public override Texture? GetCurrentTexture() => throw new NotImplementedException();
    protected override void ResizeBackBuffer() => throw new NotImplementedException();
    protected internal override void Destroy() => throw new NotImplementedException();
}
