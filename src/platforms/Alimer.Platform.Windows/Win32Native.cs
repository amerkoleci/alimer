// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using System.Runtime.InteropServices;

namespace Alimer
{
    internal static class Win32Native
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct NativeMessage
        {
            public IntPtr Hwnd;
            public uint Value;
            public IntPtr WParam;
            public IntPtr LParam;
            public uint Time;
            public Point Point;
        }

        public enum WindowMessage : uint
        {
            Null = 0x0000,
            Create = 0x0001,
            Destroy = 0x0002,
            Move = 0x0003,
            Size = 0x0005,
            Activate = 0x0006,
            SetFocus = 0x0007,
            KillFocus = 0x0008,
            Enable = 0x000A,
            SetRedraw = 0x000B,
            SetText = 0x000C,
            GetText = 0x000D,
            GetTextLength = 0x000E,
            Paint = 0x000F,
            Close = 0x0010,
            QueryEndSession = 0x0011,
            QueryOpen = 0x0013,
            EndSession = 0x0016,
            Quit = 0x0012,
            EraseBackground = 0x0014,
            SystemColorChange = 0x0015,
            ShowWindow = 0x0018,
            WindowsIniChange = 0x001A,
            SettingChange = WindowsIniChange,
            DevModeChange = 0x001B,
            ActivateApp = 0x001C,
            FontChange = 0x001D,
            TimeChange = 0x001E,
            CancelMode = 0x001F,
            SetCursor = 0x0020,
            MouseActivate = 0x0021,
            ChildActivate = 0x0022,
            KeyDown = 0x0100,
            KeyUp = 0x0101,
            Char = 0x0102,
            SysKeyDown = 0x0104,
            SysKeyUp = 0x0105,
            MouseMove = 0x0200,
            LButtonDown = 0x0201,
            LButtonUp = 0x0202,
            MButtonDown = 0x0207,
            MButtonUp = 0x0208,
            RButtonDown = 0x0204,
            RButtonUp = 0x0205,
            MouseWheel = 0x020A,
            XButtonDown = 0x020B,
            XButtonUp = 0x020C,
            MouseLeave = 0x02A3,
            NcMouseMove = 0x00A0,
            WindowPositionChanging = 0x0046,
            WindowPositionChanged = 0x0047,
        }

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetModuleHandle(string lpModuleName);

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("user32.dll", CharSet = CharSet.Unicode, EntryPoint = "PeekMessageW")]
        public static extern bool PeekMessage(out NativeMessage lpMsg, IntPtr hWnd, int wMsgFilterMin, int wMsgFilterMax, int wRemoveMsg);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        public static extern int GetMessage(out NativeMessage lpMsg, IntPtr hWnd, uint wMsgFilterMin, uint wMsgFilterMax);

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("user32.dll", ExactSpelling = true)]
        public static extern bool TranslateMessage([In] ref NativeMessage lpMsg);

        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        public static extern IntPtr DispatchMessage([In] ref NativeMessage lpmsg);
    }
}
