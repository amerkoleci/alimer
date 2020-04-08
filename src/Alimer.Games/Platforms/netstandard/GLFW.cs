// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
    public static class GLFW
    {
        public const string LIBRARY = "glfw3";
        public static readonly IntPtr NativeLibrary = GetNativeLibrary();

        private static IntPtr GetNativeLibrary()
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

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int d_glfwInit();
        public static d_glfwInit glfwInit = LoadFunction<d_glfwInit>(NativeLibrary, "glfwInit");

        private static IntPtr LoadLibrary(string libname)
        {
            var assemblyLocation = Path.GetDirectoryName(typeof(GLFW).Assembly.Location) ?? "./";
            IntPtr ret;

            // Try .NET Framework / mono locations
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                ret = LoadLibraryOS(Path.Combine(assemblyLocation, libname));

                // Look in Frameworks for .app bundles
                if (ret == IntPtr.Zero)
                    ret = LoadLibraryOS(Path.Combine(assemblyLocation, "..", "Frameworks", libname));
            }
            else
            {
                if (Environment.Is64BitProcess)
                    ret = LoadLibraryOS(Path.Combine(assemblyLocation, "x64", libname));
                else
                    ret = LoadLibraryOS(Path.Combine(assemblyLocation, "x86", libname));
            }

            // Try .NET Core development locations
            if (ret == IntPtr.Zero)
                ret = LoadLibraryOS(Path.Combine(assemblyLocation, "native", Rid, libname));

            if (ret == IntPtr.Zero)
                ret = LoadLibraryOS(Path.Combine(assemblyLocation, "runtimes", Rid, "native", libname));

            // Try current folder (.NET Core will copy it there after publish) or system library
            if (ret == IntPtr.Zero)
                ret = LoadLibraryOS(libname);

            // Welp, all failed, PANIC!!!
            if (ret == IntPtr.Zero)
                throw new Exception("Failed to load library: " + libname);

            return ret;
        }

        private static IntPtr LoadLibraryOS(string libname)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                return Windows.LoadLibraryW(libname);

            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                return OSX.dlopen(libname, RTLD_LAZY);

            return Linux.dlopen(libname, RTLD_LAZY);
        }

        private static T LoadFunction<T>(IntPtr library, string function, bool throwIfNotFound = false)
        {
            var ret = IntPtr.Zero;

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                ret = Windows.GetProcAddress(library, function);
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                ret = OSX.dlsym(library, function);
            else
                ret = Linux.dlsym(library, function);

            if (ret == IntPtr.Zero)
            {
                if (throwIfNotFound)
                    throw new EntryPointNotFoundException(function);

                return default;
            }

            return Marshal.GetDelegateForFunctionPointer<T>(ret);
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

        private const int RTLD_LAZY = 0x0001;
        private class Windows
        {
            [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

            [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
            public static extern IntPtr LoadLibraryW(string lpszLib);
        }

        private class Linux
        {
            [DllImport("libdl.so.2")]
            public static extern IntPtr dlopen(string path, int flags);

            [DllImport("libdl.so.2")]
            public static extern IntPtr dlsym(IntPtr handle, string symbol);
        }

        private class OSX
        {
            [DllImport("/usr/lib/libSystem.dylib")]
            public static extern IntPtr dlopen(string path, int flags);

            [DllImport("/usr/lib/libSystem.dylib")]
            public static extern IntPtr dlsym(IntPtr handle, string symbol);
        }
    }
}
