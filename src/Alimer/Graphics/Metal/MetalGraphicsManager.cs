// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsManager : GraphicsManager
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);
    private readonly GraphicsAdapter[] _adapters;

    /// <summary>
    /// Gets value indicating whether Metal is supported on this platform.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    /// <inheritdoc/>
    public override GraphicsBackendType BackendType => GraphicsBackendType.Metal;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public MetalGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
    }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {

    }

    private static bool CheckIsSupported()
    {
        bool result = false;
        try
        {
            if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
            {
                using NSArray allDevices = MTLCopyAllDevices();
                result |= allDevices.Count > 0;
            }
            else
            {
                using MTLDevice defaultDevice = MTLCreateSystemDefaultDevice();

                if (defaultDevice.IsNotNull)
                {
                    result = true;
                }
            }
        }
        catch
        {
            result = false;
        }

        return result;
    }
}
