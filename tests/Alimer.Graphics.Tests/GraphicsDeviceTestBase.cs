// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

public abstract class GraphicsDeviceTestBase
{
    private readonly GraphicsBackendType _backendType;
    private GraphicsDevice _graphicsDevice = null!;

    public GraphicsDevice GraphicsDevice => _graphicsDevice;

    protected GraphicsDeviceTestBase(GraphicsBackendType backendType)
    {
        _backendType = backendType;
    }

    [OneTimeSetUp]
    public void OnSetUp()
    {
        GraphicsDeviceDescription description = new()
        {
            PreferredBackend = _backendType,
#if DEBUG
            ValidationMode = ValidationMode.Enabled
#endif
        };

        _graphicsDevice = GraphicsDevice.CreateDefault(in description);
        Assert.AreEqual(_graphicsDevice.Backend, _backendType);
    }

    [OneTimeTearDown]
    public void OnTearDown()
    {
        _graphicsDevice.WaitIdle();
        _graphicsDevice.Dispose();
    }
}
