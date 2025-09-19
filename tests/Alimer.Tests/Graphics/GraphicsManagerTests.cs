// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;

namespace Alimer.Graphics.Tests;

[Trait("Graphics", "GraphicsManager")]
public class GraphicsManagerTests
{
    [Fact]
    public void DefaultManager()
    {
        GraphicsManagerOptions options = new();

        using GraphicsManager manager = GraphicsManager.Create(in options);
        if (OperatingSystem.IsWindows())
        {
            Assert.True(manager.BackendType == GraphicsBackendType.D3D12);
        }
        else if (OperatingSystem.IsAndroid() || OperatingSystem.IsLinux())
        {
            Assert.True(manager.BackendType == GraphicsBackendType.Vulkan);
        }
    }
}
