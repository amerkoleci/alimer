// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Input;

namespace Alimer;

partial class AlimerApi
{
    #region Enums
    public enum EventType
    {
        Unknown = 0,
        Quit,
        Terminating,
        LowMemory,
        WillEnterBackground,
        DidEnterBackground,
        WillEnterForeground,
        DidEnterForeground,
        LocaleChanged,
        SystemThemeChanged,

        Window,
        KeyDown,
        KeyUp,
        TextInput,

        MouseMotion,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        MouseAdded,
        MouseRemoved,

        ClipboardUpdate,

        Count,
    }

    public enum WindowEventType
    {
        None = 0,
        Shown,
        Hidden,
        Exposed,
        Moved,
        Resized,
        SizeChanged,
        Minimized,
        Maximized,
        Restored,
        Enter,
        Leave,
        FocusGained,
        FocusLost,
        CloseRequested,

        Count,
    }
    #endregion

    #region Structs
    public struct WindowIcon
    {
        public uint width;
        public uint height;
        public unsafe byte* data;
    }

    public struct WindowDesc
    {
        public unsafe byte* title;
        public uint width;
        public uint height;
        public WindowFlags flags;
        public WindowIcon icon;
    }

    public struct WindowEvent
    {
        public WindowEventType type;
        public uint windowID;
        public int data1;
        public int data2;
    }

    public struct KeyEvent
    {
        public uint windowID;
        public Keys key;
        public Bool8 alt;
        public Bool8 ctrl;
        public Bool8 shift;
        public Bool8 system;
    }

    public struct TextInputEvent
    {
        public uint windowID;
        public unsafe byte* text;
    }

    public struct MouseMotionEvent
    {
        public uint windowID;
        public float x;
        public float y;
        public float xRelative;
        public float yRelative;
    }

    public struct MouseButtonEvent
    {
        public uint windowID;
        public float x;
        public float y;
        public MouseButton button;
    }

    public struct MouseWheelEvent
    {
        public uint windowID;
        public float x;
        public float y;
    }

    public struct PlatformEvent
    {
        public EventType type;

        private Union _union;

        [UnscopedRef]
        public ref WindowEvent window
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.window;
        }

        [UnscopedRef]
        public ref KeyEvent key
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.key;
        }

        [UnscopedRef]
        public ref TextInputEvent text
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.text;
        }

        [UnscopedRef]
        public ref MouseMotionEvent motion
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.motion;
        }

        [UnscopedRef]
        public ref MouseButtonEvent button
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.button;
        }

        [UnscopedRef]
        public ref MouseWheelEvent wheel
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => ref _union.wheel;
        }

        [StructLayout(LayoutKind.Explicit)]
        public partial struct Union
        {
            [FieldOffset(0)]
            public WindowEvent window;

            [FieldOffset(0)]
            public KeyEvent key;

            [FieldOffset(0)]
            public TextInputEvent text;

            [FieldOffset(0)]
            public MouseMotionEvent motion;

            [FieldOffset(0)]
            public MouseButtonEvent button;

            [FieldOffset(0)]
            public MouseWheelEvent wheel;
        }
    }
    #endregion

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerPlatformInit();
    [LibraryImport(LibraryName)]
    public static partial void alimerPlatformShutdown();

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static unsafe partial bool alimerPlatformPollEvent(PlatformEvent* evt);

    #region Window
    [LibraryImport(LibraryName)]
    public static partial nint alimerWindowCreate(in WindowDesc desc);

    [LibraryImport(LibraryName)]
    public static partial void alimerWindowDestroy(nint window);
    [LibraryImport(LibraryName)]
    public static partial uint alimerWindowGetID(nint window);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerWindowIsOpen(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowSetPosition(nint window, int x, int y);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowGetPosition(nint window, out int x, out int y);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowSetCentered(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowSetSize(nint window, int width, int height);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowGetSize(nint window, out int width, out int height);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowGetSizeInPixels(nint window, out int width, out int height);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial void alimerWindowSetTitle(nint window, string title);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial string? alimerWindowGetTitle(nint window);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerWindowIsMinimized(nint window);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerWindowIsMaximized(nint window);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerWindowIsFullscreen(nint window);

    [LibraryImport(LibraryName)]
    public static partial void alimerWindowSetFullscreen(nint window, [MarshalAs(UnmanagedType.U1)] bool value);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerWindowHasFocus(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowShow(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowHide(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowMaximize(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowMinimize(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowRestore(nint window);
    [LibraryImport(LibraryName)]
    public static partial void alimerWindowFocus(nint window);

    [LibraryImport(LibraryName)]
    public static partial nint alimerGetWin32Window(nint window);

    [LibraryImport(LibraryName)]
    public static partial nint alimerGetCocoaWindow(nint window);
    [LibraryImport(LibraryName)]
    public static partial nint alimerGetWaylandDisplay(nint window);
    [LibraryImport(LibraryName)]
    public static partial nint alimerGetWaylandSurface(nint window);
    [LibraryImport(LibraryName)]
    public static partial nint alimerGetX11Display(nint window);
    [LibraryImport(LibraryName)]
    public static partial ulong alimerGetX11Window(nint window);
    #endregion

    #region Clipboard
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerHasClipboardText();

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial void alimerClipboardSetText(string? text);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial string? alimerClipboardGetText();
    #endregion
}
