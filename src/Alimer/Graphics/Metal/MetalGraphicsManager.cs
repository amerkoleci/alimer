// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.Versioning;
using Alimer.Platforms.Apple;
using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

//[SupportedOSPlatform("macos")]
internal class MetalGraphicsManager : GraphicsManager
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);
    private readonly MetalGraphicsAdapter[] _adapters;

    /// <summary>
    /// Gets value indicating whether Metal is supported on this platform.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public MetalGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
        if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            List<MetalGraphicsAdapter> adapters = [];
            NSArray allDevices = MTLCopyAllDevices();
            for (ulong i = 0; i < allDevices.Count; i++)
            {
                MTLDevice device = allDevices.Object<MTLDevice>(i);
                adapters.Add(new MetalGraphicsAdapter(this, device));
            }

            _adapters = [.. adapters];
        }
        else
        {
            MTLDevice defaultDevice = MTLCreateSystemDefaultDevice();
            _adapters = [new MetalGraphicsAdapter(this, defaultDevice)];
        }
    }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {

    }

    private static bool CheckIsSupported()
    {
        if (!OperatingSystem.IsMacOS() &&
            !OperatingSystem.IsMacCatalyst() &&
            !OperatingSystem.IsIOS())
        {
            return false;
        }

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
