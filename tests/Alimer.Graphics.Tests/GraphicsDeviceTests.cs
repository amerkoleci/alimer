// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(GraphicsDevice))]
[Category("General")]
public class GraphicsDeviceTests
{
    [TestCase]
    public void DefaultDevice()
    {
        GraphicsDeviceDescription description = new();

        using GraphicsDevice device = GraphicsDevice.CreateDefault(in description);
        if (OperatingSystem.IsWindows())
        {
            Assert.IsTrue(device.Backend == GraphicsBackendType.D3D12);
        }
        else if (OperatingSystem.IsAndroid() || OperatingSystem.IsLinux()) 
        {
            Assert.IsTrue(device.Backend == GraphicsBackendType.Vulkan);
        }
    }
}
