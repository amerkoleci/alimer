// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using Microsoft.UI.Xaml.Controls;
using Windows.UI.ViewManagement;
using WinRT;
using PlatformWindow = Microsoft.UI.Xaml.Window;

namespace Alimer;

internal unsafe class SwapChainPanelView : Window
{
    private readonly WindowsPlatform _platform;
    private readonly SwapChainPanel _swapChainPanel;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    /// <inheritdoc />
    public override bool IsMinimized => _minimized;

    /// <inheritdoc />
    public override bool IsFullscreen
    {
        get => _isFullscreen;
        set
        {
            if (_isFullscreen != value)
            {
                _isFullscreen = value;
                var view = ApplicationView.GetForCurrentView();
                if (view.IsFullScreenMode)
                {
                    if (!_isFullscreen)
                    {
                        view.ExitFullScreenMode();
                    }
                }
                else
                {
                    if (_isFullscreen)
                    {
                        view.TryEnterFullScreenMode();
                    }
                }
            }
        }
    }

    /// <inheritdoc />
    public override Point Position
    {
        get
        {
            return Point.Empty;
        }
        set
        {
        }
    }

    /// <inheritdoc />
    public override Size ClientSize => _clientSize;

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public override nint ContextHandle { get; }

    /// <inheritdoc />
    public override nint Handle { get; }

    public SwapChainPanelView(WindowsPlatform platform, SwapChainPanel? swapChainPanel)
    {
        _platform = platform;
        _swapChainPanel = swapChainPanel ?? new SwapChainPanel();
        _title = PlatformWindow.Current.Title;
        //CoreWindow coreWindow = Window.Current.CoreWindow;
        Kind = SwapChainSurfaceType.SwapChainPanel;
        Handle = ((IWinRTObject)_swapChainPanel).NativeObject.GetRef();
    }

    public void Show()
    {
        PlatformWindow.Current.AppWindow.Show(activateWindow: true);
    }

    protected override void SetTitle(string title)
    {
        PlatformWindow.Current.AppWindow.Title = title;
    }
}
