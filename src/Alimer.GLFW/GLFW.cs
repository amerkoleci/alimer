// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
    public static partial class GLFW
    {
        public enum InitHint
        {
            /// <summary>
            /// Joystick hat buttons init hint
            /// </summary>
            JoystickHatButtons = 0x00050001,
            CocoaChdirResources = 0x00051001,
            CocoaMenubar = 0x00051002
        }

        private const int GLFW_FALSE = 0;
        private const int GLFW_TRUE = 1;

        private static readonly ILibraryLoader s_loader = InitializeLoader();
        private static readonly IntPtr s_glfwLibrary = LoadGLFWLibrary();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void glfwVoidDelegate_t();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void glfwHintDelegate_t(int hint, int value);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int glfwInit_t();
        private static readonly glfwInit_t s_glfwInit = LoadFunction<glfwInit_t>(nameof(glfwInit));
        public static bool glfwInit() => s_glfwInit() == GLFW_TRUE;

        private static readonly glfwVoidDelegate_t s_glfwTerminate = LoadFunction<glfwVoidDelegate_t>(nameof(glfwTerminate));
        public static void glfwTerminate() => s_glfwTerminate();

        private static readonly glfwHintDelegate_t s_glfwInitHint = LoadFunction<glfwHintDelegate_t>(nameof(glfwInitHint));
        public static void glfwInitHint(InitHint hint, int value) => s_glfwInitHint((int)hint, value);

        private static readonly glfwVoidDelegate_t s_glfwPollEvents = LoadFunction<glfwVoidDelegate_t>(nameof(glfwPollEvents));
        public static void glfwPollEvents() => s_glfwPollEvents();

        #region NativeLibrary logic
        private static ILibraryLoader InitializeLoader()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return new WindowsLoader();
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return new OSXLoader();
            }
            else
            {
                return new UnixLoader();
            }
        }

        private static IntPtr LoadGLFWLibrary()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return LoadLibrary("glfw3.dll");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return LoadLibrary("libglfw3.so");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return LoadLibrary("libglfw3.dylib");
            }
            else
            {
                return LoadLibrary("glfw3");
            }
        }

        private static IntPtr LoadLibrary(string libname)
        {
            string? assemblyLocation = Path.GetDirectoryName(typeof(GLFW).Assembly.Location) ?? "./";
            IntPtr ret;

            // Try .NET Framework / mono locations
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, libname));

                // Look in Frameworks for .app bundles
                if (ret == IntPtr.Zero)
                    ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, "..", "Frameworks", libname));
            }
            else
            {
                if (Environment.Is64BitProcess)
                    ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, "x64", libname));
                else
                    ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, "x86", libname));
            }

            // Try .NET Core development locations
            if (ret == IntPtr.Zero)
                ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, "native", Rid, libname));

            if (ret == IntPtr.Zero)
                ret = s_loader.LoadLibrary(Path.Combine(assemblyLocation, "runtimes", Rid, "native", libname));

            // Try current folder (.NET Core will copy it there after publish) or system library
            if (ret == IntPtr.Zero)
                ret = s_loader.LoadLibrary(libname);

            // Welp, all failed, PANIC!!!
            if (ret == IntPtr.Zero)
                throw new Exception("Failed to load library: " + libname);

            return ret;
        }

        private static T LoadFunction<T>(string function)
        {
            IntPtr handle = s_loader.GetSymbol(s_glfwLibrary, function);

            if (handle == IntPtr.Zero)
            {
                    throw new EntryPointNotFoundException(function);
            }

            return Marshal.GetDelegateForFunctionPointer<T>(handle);
        }

        private static T? LoadOptionalFunction<T>(string function)
        {
            IntPtr handle = s_loader.GetSymbol(s_glfwLibrary, function);

            if (handle == IntPtr.Zero)
                return default;

            return Marshal.GetDelegateForFunctionPointer<T>(handle);
        }

        private static string Rid
        {
            get
            {
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) && Environment.Is64BitProcess)
                    return "win-x64";
                else if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) && !Environment.Is64BitProcess)
                    return "win-x86";
                else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
                    return "linux-x64";
                else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                    return "osx";
                else
                    return "unknown";
            }
        }

        internal interface ILibraryLoader
        {
            IntPtr LoadLibrary(string name);

            IntPtr GetSymbol(IntPtr module, string name);
        }

        private class WindowsLoader : ILibraryLoader
        {
            public IntPtr LoadLibrary(string name) => LoadLibraryW(name);

            public IntPtr GetSymbol(IntPtr module, string name) => GetProcAddress(module, name);

            [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
            private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

            [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
            private static extern IntPtr LoadLibraryW(string lpszLib);

            [DllImport("kernel32")]
            private static extern int FreeLibrary(IntPtr module);
        }

        private class UnixLoader : ILibraryLoader
        {
            private const int RTLD_LOCAL = 0x0000;
            private const int RTLD_NOW = 0x0002;

            public IntPtr LoadLibrary(string name) => dlopen(name, RTLD_NOW | RTLD_LOCAL);

            public IntPtr GetSymbol(IntPtr module, string name) => dlsym(module, name);


            [DllImport("libdl.so.2")]
            private static extern IntPtr dlopen(string path, int flags);

            [DllImport("libdl.so.2")]
            private static extern IntPtr dlsym(IntPtr handle, string symbol);
        }

        private class OSXLoader : ILibraryLoader
        {
            private const int RTLD_LOCAL = 0x0000;
            private const int RTLD_NOW = 0x0002;

            public IntPtr LoadLibrary(string name) => dlopen(name, RTLD_NOW | RTLD_LOCAL);

            public IntPtr GetSymbol(IntPtr module, string name) => dlsym(module, name);


            [DllImport("/usr/lib/libSystem.dylib")]
            private static extern IntPtr dlopen(string path, int flags);

            [DllImport("/usr/lib/libSystem.dylib")]
            private static extern IntPtr dlsym(IntPtr handle, string symbol);
        }
        #endregion
    }
}
