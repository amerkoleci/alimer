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
        GPUFactoryDesc factoryDesc = new()
        {
            preferredBackend = options.PreferredBackend,
            validationMode = options.ValidationMode
        };

        Handle = agpuCreateFactory(&factoryDesc);
        BackendType = agpuFactoryGetBackend(Handle);
        //GPUAdapter adapter = agpuFactoryGetBestAdapter(Handle);

        _adapters = new GraphicsAdapter[agpuFactoryGetAdapterCount(Handle)];
        for(int i = 0; i < _adapters.Length; i++)
        {
            GPUAdapter adapter = agpuFactoryGetAdapter(Handle, i);
            _adapters[i] = new NativeGraphicsAdapter(this, adapter);
        }
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
