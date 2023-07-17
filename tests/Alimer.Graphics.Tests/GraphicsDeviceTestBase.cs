// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Tests;

public abstract class GraphicsDeviceTestBase : IDisposable
{
    public  GraphicsBackendType BackendType { get; }

    public GraphicsDevice GraphicsDevice { get; }

    protected GraphicsDeviceTestBase(GraphicsBackendType backendType)
    {
        BackendType = backendType;
        GraphicsDeviceDescription description = new()
        {
            PreferredBackend = BackendType,
#if DEBUG
            ValidationMode = ValidationMode.Enabled
#endif
        };

        GraphicsDevice = GraphicsDevice.CreateDefault(in description);
    }

    public void Dispose()
    {
        GraphicsDevice.WaitIdle();
        GraphicsDevice.Dispose();
    }
}
