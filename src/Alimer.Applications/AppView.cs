// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;

namespace Alimer;

/// <summary>
/// Defines an application view.
/// </summary>
public abstract class AppView : ISwapChainSurface
{
    public event EventHandler? SizeChanged;

    protected AppView()
    {

    }

    public abstract bool IsMinimized { get; }
    public abstract SizeF ClientSize { get; }

    /// <inheritdoc />
    public abstract SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public abstract nint ContextHandle { get; }

    /// <inheritdoc />
    public abstract nint Handle { get; }

    /// <inheritdoc />
    SizeF ISwapChainSurface.Size => ClientSize;

    public SwapChain? SwapChain { get; private set; }
    
    public void CreateSwapChain(GraphicsDevice device)
    {
        SwapChainDescription description = new();
        SwapChain = device.CreateSwapChain(this, description);
    }

    protected virtual void OnSizeChanged()
    {
        SizeChanged?.Invoke(this, EventArgs.Empty);
    }
}
