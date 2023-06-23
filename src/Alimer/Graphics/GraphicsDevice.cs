// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using Alimer.Graphics.VGPU;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.VGPU.VGPU;
using static Alimer.Graphics.VGPU.VGPUUtils;

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

    public GraphicsDevice(in GraphicsDeviceDescription description)
    {
        VGPUDeviceDescriptor deviceDesc = new()
        {
            //deviceDesc.label = "test device";
            preferredBackend = description.PreferredBackend.ToVGPU(),
            validationMode = description.ValidationMode.ToVGPU(),
            powerPreference = description.PowerPreference.ToVGPU(),
        };

        Handle = vgpuCreateDevice(&deviceDesc);
        Guard.IsTrue(Handle.IsNotNull);

        Backend = vgpuGetBackend(Handle).FromVGPU();
    }

    internal VGPUDevice Handle { get; }

    /// <summary>
    /// Get the device backend type.
    /// </summary>
    public GraphicsBackendType Backend { get; }

    /// <summary>
    /// Gets the device validation mode.
    /// </summary>
    public ValidationMode ValidationMode { get; }

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

            vgpuDestroyDevice(Handle);
        }
    }

    /// <summary>
    /// Wait for device to finish pending GPU operations.
    /// </summary>
    public void WaitIdle() => vgpuWaitIdle(Handle);

    public static bool IsBackendSupport(GraphicsBackendType backend)
    {
        Guard.IsTrue(backend != GraphicsBackendType.Count, nameof(backend), "Invalid backend");

        return vgpuIsBackendSupported(backend.ToVGPU());
    }

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
    }
}
