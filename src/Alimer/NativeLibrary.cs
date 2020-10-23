// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer
{
    public sealed class NativeLibrary
    {
        private static readonly ILibraryLoader s_platformDefaultLoader = GetPlatformDefaultLoader();
        private readonly IntPtr _handle;

        public NativeLibrary(params string[] names)
        {
            Guard.AssertNotNull(names, nameof(names));

            for (int i = 0; i < names.Length; i++)
            {
                _handle = TryLoadLibrary(names[i]);
            }
        }

        public T LoadFunction<T>(string function, bool throwIfNotFound = false)
        {
            var handle = s_platformDefaultLoader.GetSymbol(_handle, function);

            if (handle == IntPtr.Zero)
            {
                if (throwIfNotFound)
                    throw new EntryPointNotFoundException(function);

                return default;
            }

            return Marshal.GetDelegateForFunctionPointer<T>(handle);
        }

        private IntPtr TryLoadLibrary(string name)
        {
            var assemblyLocation = Path.GetDirectoryName(typeof(NativeLibrary).Assembly.Location) ?? "./";
            IntPtr ret;

            // Try .NET Framework / mono locations
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, name));

                // Look in Frameworks for .app bundles
                if (ret == IntPtr.Zero)
                    ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, "..", "Frameworks", name));
            }
            else
            {
                if (Environment.Is64BitProcess)
                    ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, "x64", name));
                else
                    ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, "x86", name));
            }

            // Try .NET Core development locations
            if (ret == IntPtr.Zero)
                ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, "native", GetPlaformRid(), name));

            if (ret == IntPtr.Zero)
                ret = s_platformDefaultLoader.LoadLibrary(Path.Combine(assemblyLocation, "runtimes", GetPlaformRid(), "native", name));

            // Try current folder (.NET Core will copy it there after publish) or system library
            if (ret == IntPtr.Zero)
                ret = s_platformDefaultLoader.LoadLibrary(name);

            // Welp, all failed, PANIC!!!
            if (ret == IntPtr.Zero)
                throw new PlatformNotSupportedException("Failed to load library: " + name);

            return ret;
        }

        private static string GetPlaformRid()
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

        private static ILibraryLoader GetPlatformDefaultLoader()
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


        #region Nested
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
