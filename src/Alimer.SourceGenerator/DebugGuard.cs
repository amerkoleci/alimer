using System.Diagnostics;

namespace Alimer.SourceGenerators;

internal static class DebugGuard
{
    private const string LaunchDebuggerOnStartEnvVar = "ALIMER_LAUNCH_DEBUGGER_ON_START";
    private const string LaunchDebuggerOnExceptionEnvVar = "ALIMER_LAUNCH_DEBUGGER_ON_EXCEPTION";

    public static void MaybeLaunchDebuggerOnStartup()
    {
        if (GetBooleanEnvironmentVariable(LaunchDebuggerOnStartEnvVar))
        {
            Debugger.Launch();
        }
    }

    public static void Invoke<T1, T2>(this Action<T1, T2> action, T1 arg1, T2 arg2)
    {
        try
        {
            action(arg1, arg2);
        }
        catch
        {
            MaybeLaunchDebuggerOnException();
            throw;
        }
    }

    public static TResult Invoke<T1, T2, T3, TResult>(Func<T1, T2, T3, TResult> func, T1 arg1, T2 arg2, T3 arg3)
    {
        try
        {
            return func(arg1, arg2, arg3);
        }
        catch
        {
            MaybeLaunchDebuggerOnException();
            throw;
        }
    }

    private static void MaybeLaunchDebuggerOnException()
    {
        if (GetBooleanEnvironmentVariable(LaunchDebuggerOnExceptionEnvVar))
        {
            Debugger.Launch();
        }
    }

    private static bool GetBooleanEnvironmentVariable(string envVarName)
    {
#pragma warning disable RS1035 // Do not use APIs banned for analyzers
        if (Environment.GetEnvironmentVariable(envVarName) is not string value)
#pragma warning restore RS1035 // Do not use APIs banned for analyzers
        {
            return false;
        }

        return int.TryParse(value, out int intResult) ? intResult is not 0 :
            bool.TryParse(value, out bool boolResult) && boolResult;
    }
}
