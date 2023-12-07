// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;

namespace Alimer.Graphics.Tests;

[Trait("Graphics", "GraphicsDevice")]
public class GraphicsDeviceTests
{
    [Fact]
    public void DefaultDevice()
    {
        GraphicsDeviceDescription description = new();

        using GraphicsDevice device = GraphicsDevice.CreateDefault(in description);
        if (OperatingSystem.IsWindows())
        {
            Assert.True(device.Backend == GraphicsBackendType.D3D12);
        }
        else if (OperatingSystem.IsAndroid() || OperatingSystem.IsLinux())
        {
            Assert.True(device.Backend == GraphicsBackendType.Vulkan);
        }
    }
}
