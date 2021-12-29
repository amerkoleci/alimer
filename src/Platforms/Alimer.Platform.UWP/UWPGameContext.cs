// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Microsoft.Extensions.DependencyInjection;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Core;
using Windows.UI;
using Windows.UI.ViewManagement;

namespace Alimer;

internal sealed class UWPGameContext : GameContextWithGraphics, IFrameworkViewSource
{
    private bool _isActive;
    private readonly UWPGameView _mainView;

    public UWPGameContext(GraphicsDevice graphicsDevice)
        : base(graphicsDevice)
    {
        CoreApplication.Resuming += OnCoreApplicationResuming;
        CoreApplication.Suspending += OnCoreApplicationSuspending;

        View = _mainView = new UWPGameView (this);
    }

    // <inheritdoc />
    public override GameView View { get; }

    public override void ConfigureServices(IServiceCollection services)
    {
        base.ConfigureServices(services);
    }

    // <inheritdoc />
    //public override bool IsActive => _isActive;

    IFrameworkView IFrameworkViewSource.CreateView() => _mainView;

    private void OnCoreApplicationResuming(object sender, object e)
    {
        //OnResume();
    }

    private void OnCoreApplicationSuspending(object sender, SuspendingEventArgs e)
    {
        SuspendingDeferral deferral = e.SuspendingOperation.GetDeferral();

        //using (var device3 = game.GraphicsDevice.NativeDevice.QueryInterface<SharpDX.DXGI.Device3>())
        //{
        //    game.GraphicsContext.CommandList.ClearState();
        //    device3.Trim();
        //}

        //OnSuspend();

        deferral.Complete();
    }

    public static void ExtendViewIntoTitleBar(bool extendViewIntoTitleBar)
    {
        CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = extendViewIntoTitleBar;

        if (extendViewIntoTitleBar)
        {
            ApplicationViewTitleBar titleBar = ApplicationView.GetForCurrentView().TitleBar;
            titleBar.ButtonBackgroundColor = Colors.Transparent;
            titleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
        }
    }

    // <inheritdoc />
    public override void RunMainLoop(Action init, Action callback)
    {
        CoreApplication.Run(this);
    }

#if TODO
    // <inheritdoc />
    public override void RequestExit()
    {
        //ExitRequested = true;
        //OnExiting();
        CoreApplication.Exit();
        //Application.Current.Exit();
    } 
#endif

    public void Activate()
    {
        _isActive = true;
        //OnActivated();
    }

    public void Suspend()
    {
        //OnSuspend();
    }

    // https://docs.microsoft.com/en-us/uwp/api/windows.ui.viewmanagement.applicationview.preferredlaunchwindowingmode?view=winrt-22000
    private void ToggleFullScreenModeButtonTest()
    {
        var view = ApplicationView.GetForCurrentView();
        if (view.IsFullScreenMode)
        {
            view.ExitFullScreenMode();
            ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.Auto;
            // The SizeChanged event will be raised when the exit from full-screen mode is complete.
        }
        else
        {
            if (view.TryEnterFullScreenMode())
            {
                ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.FullScreen;
                // The SizeChanged event will be raised when the entry to full-screen mode is complete.
            }
        }

    }
}
