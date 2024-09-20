// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Input;

namespace Alimer;

/// <summary>
/// Core application platform Module, used for managing Views, Monitors, Input and Settings.
/// </summary>
public abstract class AppPlatform : DisposableObject
{
    public Action? Ready;
    public event EventHandler? TickRequested;

    /// <summary>
    /// Gets whether the multiple views are supported.
    /// </summary>
    public abstract bool SupportsMultipleViews { get; }

    /// <summary>
    /// Gets the main window.
    /// </summary>
    public abstract Window MainWindow { get; }

    /// <summary>
    /// Gets the input manager.
    /// </summary>
    public abstract InputManager Input { get; }

    protected AppPlatform()
    {
    }

    public static AppPlatform CreateDefault()
    {
#if WINDOWS || WINDOWS_UWP
        return new WindowsPlatform(swapChainPanel: default);
#elif __ANDROID__
        throw new NotImplementedException();
#else
        return new SDLPlatform();
#endif
    }

    public abstract void RunMainLoop();
    public abstract void RequestExit();

    protected internal void OnReady()
    {
        Ready?.Invoke();
    }

    protected internal void OnTick()
    {
        TickRequested?.Invoke(this, EventArgs.Empty);
    }

    /// <summary>
    /// The User Directory safe location to store save data or preferences
    /// </summary>
    public virtual string UserDirectory(string applicationName) => DefaultUserDirectory(applicationName);

    /// <summary>
    /// Gets the Default UserDirectory
    /// </summary>
    internal static string DefaultUserDirectory(string applicationName)
    {
        if (OperatingSystem.IsWindows())
        {
            return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), applicationName);
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            string? result = Environment.GetEnvironmentVariable("HOME");
            if (!string.IsNullOrEmpty(result))
                return Path.Combine(result, "Library", "Application Support", applicationName);
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux) ||
                 RuntimeInformation.IsOSPlatform(OSPlatform.FreeBSD))
        {
            string? result = Environment.GetEnvironmentVariable("XDG_DATA_HOME");
            if (!string.IsNullOrEmpty(result))
            {
                return Path.Combine(result, applicationName);
            }
            else
            {
                result = Environment.GetEnvironmentVariable("HOME");
                if (!string.IsNullOrEmpty(result))
                    return Path.Combine(result, ".local", "share", applicationName);
            }
        }

        return AppContext.BaseDirectory;
    }
}
