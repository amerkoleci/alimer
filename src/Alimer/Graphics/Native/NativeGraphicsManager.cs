// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using System.Runtime.InteropServices;
using System.Diagnostics;
using static Alimer.Graphics.Native.AlimerGPUApi;
using System.Runtime.InteropServices.Marshalling;

namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsManager : GraphicsManager
{
    private readonly GPUFactory _handle;
    private readonly NativeGraphicsAdapter[] _adapters;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    static NativeGraphicsManager()
    {
        var level = agpuGetLogLevel();
        //agpuSetLogLevel(Level);
        agpuSetLogCallback(&OnNativeLogCallback, 0);
    }

    public NativeGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        GPUFactoryDesc factoryDesc = new()
        {
            preferredBackend = options.PreferredBackend,
            validationMode = options.ValidationMode
        };

        Handle = agpuCreateFactory(&factoryDesc);
        BackendType = agpuFactoryGetBackend(Handle);
        //GPUAdapter adapter = agpuFactoryGetBestAdapter(Handle);

        _adapters = new NativeGraphicsAdapter[agpuFactoryGetAdapterCount(Handle)];
        for (int i = 0; i < _adapters.Length; i++)
        {
            GPUAdapter adapter = agpuFactoryGetAdapter(Handle, i);
            _adapters[i] = new NativeGraphicsAdapter(this, adapter);
        }
    }

    public GPUFactory Handle { get; }
    public GraphicsBackend BackendType { get; }

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            agpuFactoryDestroy(Handle);
        }
    }

    protected override Surface CreateSurfaceCore(in SurfaceDescriptor descriptor)
    {
        throw new NotImplementedException();
    }

    [UnmanagedCallersOnly]
    private static unsafe void OnNativeLogCallback(GPULogLevel level, byte* messagePtr, nint userData)
    {
        string message = Utf8StringMarshaller.ConvertToManaged(messagePtr)!;
    }
}
