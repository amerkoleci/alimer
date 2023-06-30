// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;

namespace Alimer;

/// <summary>
/// Defines an application view.
/// </summary>
public abstract class AppView
{
    public event EventHandler? SizeChanged;

    public abstract bool IsMinimized { get; }
    public abstract Size ClientSize { get; }

    public abstract SwapChainSurface Surface { get; }
    
    public SwapChain? SwapChain { get; private set; }
    
    public void CreateSwapChain(GraphicsDevice device)
    {
        SwapChainDescription descriptor = new(ClientSize.Width, ClientSize.Height);
        SwapChain = device.CreateSwapChain(Surface, descriptor);
    }

    protected virtual void OnSizeChanged()
    {
        SizeChanged?.Invoke(this, EventArgs.Empty);
    }
}
