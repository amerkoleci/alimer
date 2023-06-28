// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(GraphicsDevice))]
public class GraphicsDeviceTests
{
    [TestCase]
    public void DefaultDevice()
    {
        GraphicsDeviceDescription description = new()
        {
#if DEBUG
            ValidationMode = ValidationMode.Enabled
#endif
        };

        using GraphicsDevice device = GraphicsDevice.CreateDefault(in description);
        if (OperatingSystem.IsWindows())
        {
            Guard.IsTrue(device.Backend == GraphicsBackendType.D3D12);
        }
        else if (OperatingSystem.IsAndroid() || OperatingSystem.IsLinux()) 
        {
            Guard.IsTrue(device.Backend == GraphicsBackendType.Vulkan);
        }
    }
}
