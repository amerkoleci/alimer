// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using Alimer.Graphics;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.ViewManagement;

namespace Alimer;

internal class UWPGameView : GameView, IFrameworkView
{
    private readonly UWPGameContext _platform;
    private bool _windowClosed;

    public UWPGameView(UWPGameContext platform)
    {
        _platform = platform;
    }

    /// <inheritdoc />
    public override SizeF ClientSize { get; }

    /// <inheritdoc />
    public override SwapChainSource Source { get; }

    private void OnApplicationViewActivated(CoreApplicationView sender, IActivatedEventArgs e)
    {
        CoreWindow.GetForCurrentThread().Activate();
        _platform.Activate();
    }

    void IFrameworkView.Initialize(CoreApplicationView applicationView)
    {
        //ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.FullScreen;
        applicationView.Activated += OnApplicationViewActivated;
    }

    void IFrameworkView.SetWindow(CoreWindow window)
    {
        window.SizeChanged += OnCoreWindowSizeChanged;
        window.Closed += OnCoreWindowClosed;
        //UWPPlatform.ExtendViewIntoTitleBar(true);
    }

    private void OnCoreWindowSizeChanged(CoreWindow sender, WindowSizeChangedEventArgs e)
    {
        OnSizeChanged();
    }

    private void OnCoreWindowClosed(CoreWindow sender, CoreWindowEventArgs args)
    {
        _windowClosed = true;
    }

    void IFrameworkView.Load(string entryPoint)
    {
    }

    void IFrameworkView.Run()
    {
        ApplicationView applicationView = ApplicationView.GetForCurrentView();
        applicationView.Title = "Alimer";

        //_platform.OnInit();

        while (!_windowClosed)
        {
            CoreWindow.GetForCurrentThread().Dispatcher.ProcessEvents(CoreProcessEventsOption.ProcessAllIfPresent);

            //_platform.Tick();
        }

        //_platform.Destroy();
    }

    void IFrameworkView.Uninitialize()
    {
    }
}
