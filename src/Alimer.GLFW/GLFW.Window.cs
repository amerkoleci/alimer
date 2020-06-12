// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
    public static partial class GLFW
    {
        private const int GLFW_CLIENT_API = 0x00022001;
        private const int GLFW_CONTEXT_ROBUSTNESS = 0x00022005;
        private const int GLFW_OPENGL_PROFILE = 0x00022008;
        private const int GLFW_CONTEXT_CREATION_API = 0x0002200B;
        private const int GLFW_CONTEXT_RELEASE_BEHAVIOR = 0x00022009;

        public enum WindowBoolHint
        {
            Focused = 0x00020001,
            Iconified = 0x00020002,
            Resizable = 0x00020003,
            Visible = 0x00020004,
            Decorated = 0x00020005,
            Iconify = 0x00020006,
            Floating = 0x00020007,
            Maximized = 0x00020008,
            CenterCursor = 0x00020009,
            TransparentFramebuffer = 0x0002000A,
            Hovered = 0x0002000B,
            FocusOnShow = 0x0002000C,
            Stereo = 0x0002100C,
            SrgbCapable = 0x0002100E,
            DoubleBuffer = 0x00021010,
            OpenGLForwardCompat = 0x00022006,
            OpenGLDebugContext = 0x00022007,
            ContextNoError = 0x0002200A,
            ScaleToMonitor = 0x0002200C,
            CocoaRetinaFramebuffer = 0x00023001,
            CocoaGraphicsSwitching = 0x00023003,
            Win32KeyboardMenu = 0x00025001
        }

        public enum WindowIntHint
        {
            RedBits = 0x00021001,
            GreenBits = 0x00021002,
            BlueBits = 0x00021003,
            AlphaBits = 0x00021004,
            DepthBits = 0x00021005,
            StencilBits = 0x00021006,

            AccumRedBits = 0x00021007,
            AccumGreenBits = 0x00021008,
            AccumBlueBits = 0x00021009,
            AccumAlphaBits = 0x0002100A,
            AuxBuffers = 0x0002100B,
            Samples = 0x0002100D,
            RefreshRate = 0x0002100F,
            ContextVersionMajor = 0x00022002,
            ContextVersionMinor = 0x00022003,
            ContextRevision = 0x00022004,
        }

        public enum WindowStringHint
        {
            CocoaFrameName = 0x00023002,
            X11ClassName = 0x00024001,
            X11InstanceName = 0x00024002,
        }

        public enum WindowClientApi
        {
            None = 0,
            OpenGL = 0x00030001,
            OpenGLES = 0x00030002
        }

        public enum WindowContextRobustness
        {
            None = 0,
            NoResetNotification = 0x00031001,
            LoseContextOnReset = 0x00031002
        }

        public enum WindowContextCreationApi
        {
            NativeContext = 0x00036001,
            EglContext = 0x00036002,
            OSMesaContext = 0x00036003,
        }

        public enum WindowContextReleaseBehavior
        {
            Any = 0,
            ReleaseBehaviorFlush = 0x00035001,
            ReleaseBehaviorNone = 0x00035002,
        }

        public enum OpenGLProfile
        {
            AnyProfile = 0,
            CoreProfile = 0x00032001,
            CompatProfile = 0x00032002
        }

        private static glfwVoidDelegate_t s_glfwDefaultWindowHints = LoadFunction<glfwVoidDelegate_t>(nameof(glfwDefaultWindowHints));
        public static void glfwDefaultWindowHints() => s_glfwDefaultWindowHints();

        private static glfwHintDelegate_t s_glfwWindowHint = LoadFunction<glfwHintDelegate_t>(nameof(glfwWindowHint));
        public static void glfwWindowHint(WindowBoolHint hint, bool value) => s_glfwWindowHint((int)hint, value ? GLFW_TRUE : GLFW_FALSE);
        public static void glfwWindowHint(WindowIntHint hint, int value) => s_glfwWindowHint((int)hint, value);
        public static void glfwWindowHint(WindowClientApi value) => s_glfwWindowHint(GLFW_CLIENT_API, (int)value);
        public static void glfwWindowHint(WindowContextRobustness value) => s_glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, (int)value);
        public static void glfwWindowHint(OpenGLProfile value) => s_glfwWindowHint(GLFW_OPENGL_PROFILE, (int)value);
        public static void glfwWindowHint(WindowContextCreationApi value) => s_glfwWindowHint(GLFW_CONTEXT_CREATION_API, (int)value);
        public static void glfwWindowHint(WindowContextReleaseBehavior value) => s_glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, (int)value);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void glfwStringHintDelegate_t(int hint, string value);
        private static glfwStringHintDelegate_t s_glfwWindowHintString = LoadFunction<glfwStringHintDelegate_t>("glfwWindowHintString");
        public static void glfwWindowHint(WindowStringHint hint, string value) => s_glfwWindowHintString((int)hint, value);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr glfwCreateWindow_t(int width, int height, string title, IntPtr monitor, IntPtr share);
        private static glfwCreateWindow_t s_glfwCreateWindow = LoadFunction<glfwCreateWindow_t>(nameof(glfwCreateWindow));
        public static IntPtr glfwCreateWindow(int width, int height, string title, IntPtr monitor, IntPtr share) => s_glfwCreateWindow(width, height, title, monitor, share);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int glfwWindowShouldClose_t(IntPtr window);
        private static glfwWindowShouldClose_t s_glfwWindowShouldClose = LoadFunction<glfwWindowShouldClose_t>(nameof(glfwWindowShouldClose));
        public static bool glfwWindowShouldClose(IntPtr window) => s_glfwWindowShouldClose(window) == GLFW_TRUE;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void glfwSwapBuffers_t(IntPtr window);
        private static glfwSwapBuffers_t s_glfwSwapBuffers = LoadFunction<glfwSwapBuffers_t>(nameof(glfwSwapBuffers));
        public static void glfwSwapBuffers(IntPtr window) => s_glfwSwapBuffers(window);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr glfwGetNativeHandleDelegate_t(IntPtr window);
        private static glfwGetNativeHandleDelegate_t s_glfwGetWin32Window = LoadFunction<glfwGetNativeHandleDelegate_t>(nameof(glfwGetWin32Window));
        public static IntPtr glfwGetWin32Window(IntPtr window) => s_glfwGetWin32Window(window);
    }
}
