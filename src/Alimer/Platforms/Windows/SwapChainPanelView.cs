// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Windows.UI.Core;

namespace Alimer;

internal unsafe class SwapChainPanelView : GameView
{
    private readonly WindowsPlatform _platform;
    private readonly SwapChainPanel _swapChainPanel;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly nint Handle;
    public readonly uint Id;

    /// <inheritdoc />
    public override bool IsMinimized => _minimized;

    /// <inheritdoc />
    public override Size ClientSize => _clientSize;

    /// <inheritdoc />
    //public override SwapChainSurface Surface { get; }

    public SwapChainPanelView(WindowsPlatform platform, SwapChainPanel swapChainPanel)
    {
        _platform = platform;
        _swapChainPanel = swapChainPanel;
        //CoreWindow coreWindow = Window.Current.CoreWindow;
        //Surface = SwapChainSurface.CreateSwapChainPanel(swapChainPanel);
    }

    public void Show()
    {
        Window.Current.AppWindow.Show(activateWindow: true);
    }
}
