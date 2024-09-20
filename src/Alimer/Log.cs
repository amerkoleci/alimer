// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer;

public static class Log
{
    private static readonly StringBuilder s_log = new();
    private static readonly LogAttribute[] s_logAttributes =
    [
        new("TRACE ", LogColor.White),
        new("DEBUG ", LogColor.Cyan),
        new("INFO  ", LogColor.Green),
        new("WARN  ", LogColor.Yellow),
        new("ERROR ", LogColor.Red),
        new("FATAL ", LogColor.Magenta),
        new("SYSTEM", LogColor.Cyan),
        new("ASSERT", LogColor.Magenta),
    ];
    private static readonly bool s_colorEnabled;

#if DEBUG
    public static LogLevel Level { get; set; } = LogLevel.Debug;
#else
    public static LogLevel Level { get; set; } = LogLevel.Info;
#endif
    public static bool PrintToConsole { get; set; } = true;

    static Log()
    {
        if (OperatingSystem.IsWindows())
        {
            nint stdOut = Kernel32.GetStdHandle(Kernel32.STD_OUTPUT_HANDLE);

            s_colorEnabled = Kernel32.GetConsoleMode(stdOut, out uint outConsoleMode) &&
                           Kernel32.SetConsoleMode(stdOut, outConsoleMode | Kernel32.ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        else
        {
            s_colorEnabled = false;
        }

        Info($"Logging Enabled ({Level})");
    }

    private static void LogInternalIf(
        bool condition,
        LogLevel logLevel,
        string message,
        string callerFilePath,
        int callerLineNumber)
    {
        if (condition)
        {
            LogInternal(logLevel, message, callerFilePath, callerLineNumber);
        }
    }

    private static void LogInternal(LogLevel logLevel, string message)
    {
        if (logLevel < Level)
            return;

        LogAttribute logAttribute = s_logAttributes[(int)logLevel];

        if (PrintToConsole)
        {
            Console.WriteLine(
                s_colorEnabled
                    ? $"\u001b[{LogColor.Gray}m{DateTime.Now:HH:mm:ss} \u001b[{logAttribute.Color}m[{logAttribute.Name}] \u001b[{LogColor.White}m{message}\u001b[0m"
                    : $"{DateTime.Now:HH:mm:ss} {logLevel} {message}");
        }

        s_log.Append($"{DateTime.Now.ToString("HH:mm:ss")} [{logAttribute.Name}] {message}");

        if ((logLevel == LogLevel.Error) || (logLevel == LogLevel.Assert))
        {
            Debugger.Break();
        }
    }

    private static void LogInternal(LogLevel logLevel, string message, string callerFilePath, int callerLineNumber)
    {
        if (logLevel < Level)
            return;

        LogAttribute logAttribute = s_logAttributes[(int)logLevel];
        string callSite = $"{Path.GetFileName(callerFilePath)}:{callerLineNumber}";

        if (PrintToConsole)
        {
            Console.WriteLine(
                s_colorEnabled
                    ? $"\u001b[{LogColor.Gray}m{DateTime.Now:HH:mm:ss} \u001b[{logAttribute.Color}m[{logAttribute.Name}]\u001b[{LogColor.Gray}m {callSite,-32} \u001b[{LogColor.White}m{message}\u001b[0m"
                    : $"{DateTime.Now:HH:mm:ss} {logLevel} {callSite,-32} {message}");
        }

        s_log.Append($"{DateTime.Now.ToString("HH:mm:ss")} [{logAttribute.Name}] {callSite,-32} {message}");

        if ((logLevel == LogLevel.Error) || (logLevel == LogLevel.Assert))
        {
            Debugger.Break();
        }
    }

    public static bool TraceIf(
        bool condition,
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternalIf(condition, LogLevel.Trace, message, callerFilePath, callerLineNumber);
        return condition;
    }

    public static void Trace(
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternal(LogLevel.Trace, message, callerFilePath, callerLineNumber);
    }

    public static bool DebugIf(
        bool condition,
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternalIf(condition, LogLevel.Debug, message, callerFilePath, callerLineNumber);
        return condition;
    }

    public static void Debug(
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternal(LogLevel.Debug, message, callerFilePath, callerLineNumber);
    }

    public static bool InfoIf(
        bool condition,
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternalIf(condition, LogLevel.Info, message, callerFilePath, callerLineNumber);
        return condition;
    }

    public static bool InfoIf(
        bool condition,
        object? message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        string? str;
        if (message == null)
            str = "null";
        else
        {
            str = message.ToString();
            if (str == null)
                str = "null";
        }

        LogInternalIf(condition, LogLevel.Info, str, callerFilePath, callerLineNumber);
        return condition;
    }

    public static void Info(string message)
    {
        LogInternal(LogLevel.Info, message);
    }

    public static void Info(object? message)
    {
        string? str;
        if (message == null)
        {
            str = "null";
        }
        else
        {
            str = message.ToString();
            str ??= "null";
        }

        LogInternal(LogLevel.Info, str);
    }

    public static bool WarnIf(
        bool condition,
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternalIf(condition, LogLevel.Warn, message, callerFilePath, callerLineNumber);
        return condition;
    }

    public static void Warn(string message)
    {
        LogInternal(LogLevel.Warn, message);
    }

    public static bool ErrorIf(
        [DoesNotReturnIf(true)] bool condition,
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternalIf(condition, LogLevel.Error, message, callerFilePath, callerLineNumber);
        return condition;
    }

    public static void Error(
        string message,
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        LogInternal(LogLevel.Error, message, callerFilePath, callerLineNumber);
    }

    [Conditional("DEBUG")]
    public static void Assert(
        [DoesNotReturnIf(false)] bool condition,
        string message = "Assertion failed.",
        [CallerFilePath] string callerFilePath = "",
        [CallerLineNumber] int callerLineNumber = 0)
    {
        if (!condition)
        {
            LogInternal(LogLevel.Assert, message, callerFilePath, callerLineNumber);
            throw new Exception(message);
        }
    }

    public static void WriteToFile(string file)
    {
        string? directory = Path.GetDirectoryName(file);
        if (directory != null && !Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }

        File.WriteAllText(file, s_log.ToString());
    }

    public enum LogLevel
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical,
        Assert,
    }

    private static class LogColor
    {
        public const string Black = "30";
        public const string DarkBlue = "34";
        public const string DarkGreen = "32";
        public const string DarkCyan = "36";
        public const string DarkRed = "31";
        public const string DarkMagenta = "35";
        public const string DarkYellow = "33";
        public const string Gray = "37";
        public const string DarkGray = "90";
        public const string Blue = "94";
        public const string Green = "92";
        public const string Cyan = "96";
        public const string Red = "91";
        public const string Magenta = "95";
        public const string Yellow = "93";
        public const string White = "97";
    }

    private record struct LogAttribute(string Name, string Color);
}

internal partial class Kernel32
{
    private const string LibraryName = "kernel32.dll";

    internal const int ENABLE_PROCESSED_INPUT = 0x0001;
    internal const uint ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004;
    internal const int STD_OUTPUT_HANDLE = -11;

    [LibraryImport(LibraryName)]
    [SuppressGCTransition]
    internal static partial nint GetStdHandle(int nStdHandle);

    [LibraryImport(LibraryName, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool GetConsoleMode(nint handle, out uint mode);

    [LibraryImport(LibraryName, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool SetConsoleMode(nint handle, uint mode);
}
