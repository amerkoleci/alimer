// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal class NativeGraphicsManager : GraphicsManager
{
    private readonly GraphicsAdapter[] _adapters;

    public static bool IsSupported => true;

    /// <inheritdoc/>
    public override GraphicsBackendType BackendType { get; }

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public unsafe NativeGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        Handle = agpuCreateFactory(null);
        BackendType = agpuFactoryGetBackend(Handle);
        GPUAdapter adapter = agpuFactoryRequestAdapter(Handle, null);

        _adapters = new GraphicsAdapter[1];
        _adapters[0] = new NativeGraphicsAdapter(this, adapter);
    }

    public GPUFactory Handle { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            agpuFactoryDestroy(Handle);
        }
    }
}
