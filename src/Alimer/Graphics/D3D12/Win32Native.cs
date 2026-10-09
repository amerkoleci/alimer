// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Vortice.Win32;
using Vortice.Win32.Security;

namespace Alimer.Graphics.D3D12;

internal static unsafe partial class Win32Native
{
    public const int GENERIC_ALL = 0x10000000;

    public const uint INFINITE = 0xFFFFFFFF;

    [LibraryImport("kernel32")]
    [SuppressGCTransition]
    public static partial uint GetLastError();

    [LibraryImport("kernel32")]
    public static partial Bool32 CloseHandle(HANDLE hObject);

    [LibraryImport("user32")]
    public static partial Bool32 GetWindowRect(nint hWnd, Vortice.Win32.Numerics.Rect* lpRect);

    [LibraryImport("kernel32")]
    public static partial HANDLE CreateEventW(SECURITY_ATTRIBUTES* lpEventAttributes, Bool32 bManualReset, Bool32 bInitialState, char* lpName);

    [LibraryImport("kernel32")]
    public static partial Bool32 RegisterWaitForSingleObject(HANDLE* phNewWaitObject, HANDLE hObject, delegate* unmanaged<void*, byte, void> Callback, void* Context, uint dwMilliseconds, uint dwFlags);

    [LibraryImport("kernel32")]
    public static partial Bool32 UnregisterWait(HANDLE WaitHandle);

}
