// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Input;

namespace Alimer;

internal abstract partial class RuntimePlatform
{
    public static RuntimePlatform Current
    {
        get
        {
            field ??= CreateDefault();

            return field;
        }
        set;
    }

    protected RuntimePlatform()
    {
    }

    /// <summary>
    /// Gets the main window.
    /// </summary>
    public abstract Window MainWindow { get; }

    /// <summary>
    /// Gets the platform input manager.
    /// </summary>
    public abstract InputManager Input { get; }

    public static partial RuntimePlatform CreateDefault();

    public abstract void RunMainLoop(Action ready, Action tick);
    public abstract void RequestExit();
    public abstract void Destroy();

    #region Clipboard
    protected internal abstract bool HasClipboardText();
    protected internal abstract string? GetClipboardText();
    protected internal abstract void SetClipboardText(string? text);
    #endregion

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
