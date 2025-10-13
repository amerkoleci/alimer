// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(GraphicsManager))]
public class GraphicsManagerTests
{
    [Test]
    public void DefaultManager()
    {
        GraphicsManagerOptions options = new();

        using GraphicsManager manager = GraphicsManager.Create(in options);
        if (OperatingSystem.IsWindows())
        {
            Assert.That(() => manager.BackendType, Is.EqualTo(GraphicsBackendType.D3D12));
        }
        else if (OperatingSystem.IsAndroid() || OperatingSystem.IsLinux())
        {
            Assert.That(() => manager.BackendType, Is.EqualTo(GraphicsBackendType.Vulkan));
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst() || OperatingSystem.IsIOS())
        {
            Assert.That(() => manager.BackendType, Is.EqualTo(GraphicsBackendType.Metal));
        }
    }
}
