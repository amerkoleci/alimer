// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Tests;

public abstract class GraphicsDeviceTestBase : IDisposable
{
    public  GraphicsBackend BackendType { get; }

    public GraphicsManager Manager { get; }
    public GraphicsAdapter Adapter { get; }
    public GraphicsDevice Device { get; }

    protected GraphicsDeviceTestBase(GraphicsBackend backendType)
    {
        BackendType = backendType;

        Manager = GraphicsManager.Create(new GraphicsManagerOptions
        {
            PreferredBackend = BackendType,
#if DEBUG
            ValidationMode = GraphicsValidationMode.Enabled
#endif
        });
        Adapter = Manager.GetBestAdapter();

        Device = Adapter.CreateDevice();
    }

    public void Dispose()
    {
        Device.WaitIdle();
        Device.Dispose();
        Manager.Dispose();
    }
}
