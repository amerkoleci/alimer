// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Logging;
using Avalonia.Vulkan;

namespace Alimer.Editor;

static class Program
{
    // Initialization code. Don't use any Avalonia, third-party APIs or any
    // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
    // yet and stuff might break.
    [STAThread]
    public static void Main(string[] args)
    {
        AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

        try
        {
            BuildAvaloniaApp()
                .StartWithClassicDesktopLifetime(args, ShutdownMode.OnMainWindowClose);
        }
        catch (Exception ex)
        {
            //HandleException(ex, CrashLocation.Main);
        }
    }

    // Avalonia configuration, don't remove; also used by visual designer.
    public static AppBuilder BuildAvaloniaApp()
    {
        bool preferOpenGL = false;

        return AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .WithInterFont()
            .With(new Win32PlatformOptions()
            {
                RenderingMode = preferOpenGL ? [Win32RenderingMode.Wgl, Win32RenderingMode.Vulkan] : [Win32RenderingMode.Vulkan, Win32RenderingMode.Wgl],
                OverlayPopups = true,
            })
            .With(new X11PlatformOptions()
            {
                RenderingMode = preferOpenGL ? [X11RenderingMode.Glx, X11RenderingMode.Vulkan] : [X11RenderingMode.Vulkan, X11RenderingMode.Glx],
                OverlayPopups = true,
            })
            .With(new SkiaOptions()
            {
                MaxGpuResourceSizeBytes = 1024 * 600 * 4 * 12 * 4 // quadruple the default size
            })
            .With(new VulkanOptions()
            {
                VulkanInstanceCreationOptions = new VulkanInstanceCreationOptions()
                {
                    UseDebug = true
                }
            })
            .LogToTrace()
#if DEBUG
            .LogToTrace(LogEventLevel.Verbose, "Vulkan")
#else
            .LogToTrace(LogEventLevel.Debug, "Vulkan")
#endif
            ;
    }

    private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        if (e.IsTerminating)
        {
            //HandleException(e.ExceptionObject as Exception, CrashLocation.UnhandledException);
        }
    }
}
