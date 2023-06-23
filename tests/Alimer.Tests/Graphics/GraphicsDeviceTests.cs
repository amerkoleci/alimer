// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(GraphicsDevice))]
public class GraphicsDeviceTests
{
    [TestCase]
    public void DefaultDevice()
    {
        using GraphicsDevice device = new GraphicsDevice();
    }
}
