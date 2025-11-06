// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Editor.ViewModels;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Rendering;

namespace Alimer.Editor.Views;

public partial class MainWindow : Avalonia.Controls.Window
{
    public static MainWindow? Current
    {
        get
        {
            if (Application.Current is null)
                return null;

            if (Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
                return desktop.MainWindow as MainWindow;

            throw new NotSupportedException("ApplicationLifetime is not supported");
        }
    }

    public new MainViewModel? DataContext
    {
        get => (MainViewModel?)base.DataContext;
        set => base.DataContext = value;
    }

    public MainWindow()
    {
        InitializeComponent();
#if DEBUG
        this.AttachDevTools();
        RendererDiagnostics.DebugOverlays = RendererDebugOverlays.Fps;
#endif
    }
}
