// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using Alimer.Graphics.VGPU;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.VGPU.VGPU;

using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics;

public sealed unsafe class GraphicsDevice : GraphicsObjectBase
{
    private static VGPULogCallback _logCallback;

    static GraphicsDevice()
    {
        _logCallback = OnLogNative;
        vgpuSetLogCallback(_logCallback, 0);
    }

    public GraphicsDevice()
    {
        VGPUDeviceDescriptor deviceDesc = new();
        //deviceDesc.label = "test device";
#if DEBUG
        deviceDesc.validationMode = VGPUValidationMode.Enabled;
#endif

        Handle = vgpuCreateDevice(&deviceDesc);
        Guard.IsTrue(Handle.IsNotNull);
    }

    internal VGPUDevice Handle { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="GraphicsDevice" /> class.
    /// </summary>
    ~GraphicsDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            WaitIdle();
        }
    }

    /// <summary>
    /// Wait for device to finish pending GPU operations.
    /// </summary>
    public void WaitIdle() => vgpuWaitIdle(Handle);

#if NET6_0_OR_GREATER
    //[UnmanagedCallersOnly]
#else
    [MonoPInvokeCallback(typeof(VGPULogCallbackDelegate))]
#endif
    private static void OnLogNative(VGPULogLevel level, sbyte* pMessage, nint userData)
    {
        string message = GetUtf8Span(pMessage).GetString()!;

        switch (level)
        {
            case VGPULogLevel.Error:
                Log.Error(message);
                break;
            case VGPULogLevel.Warning:
                Log.Warn(message);
                break;
            case VGPULogLevel.Info:
                Log.Info(message);
                break;
        }

        //if (_logCallback != null)
        //{
        //    _logCallback(level, new string(message));
        //}
    }
}
