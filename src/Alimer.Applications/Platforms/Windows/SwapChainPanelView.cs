// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using WinRT;

namespace Alimer;

internal unsafe class SwapChainPanelView : AppView
{
    private readonly WindowsPlatform _platform;
    private readonly SwapChainPanel _swapChainPanel;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly uint Id;

    /// <inheritdoc />
    public override bool IsMinimized => _minimized;

    /// <inheritdoc />
    public override SizeF ClientSize => _clientSize;

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public override nint ContextHandle { get; }

    /// <inheritdoc />
    public override nint Handle { get; }

    public SwapChainPanelView(WindowsPlatform platform, SwapChainPanel swapChainPanel)
    {
        _platform = platform;
        _swapChainPanel = swapChainPanel;
        _title = Window.Current.Title;
        //CoreWindow coreWindow = Window.Current.CoreWindow;
        Kind = SwapChainSurfaceType.SwapChainPanel;
        Handle = ((IWinRTObject)_swapChainPanel).NativeObject.GetRef();
    }

    public void Show()
    {
        Window.Current.AppWindow.Show(activateWindow: true);
    }

    protected override void SetTitle(string title)
    {
        Window.Current.Title = title;
    }
}
