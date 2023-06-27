// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics.VGPU;

partial class VGPU
{
    private static readonly ILibraryLoader _loader = GetPlatformLoader();
    private static IntPtr s_vgpuModule;

    static VGPU()
    {
        if (OperatingSystem.IsWindows())
        {
            s_vgpuModule = _loader.LoadNativeLibrary("vgpu.dll");

        }
        else if (OperatingSystem.IsMacOS())
        {
            s_vgpuModule = _loader.LoadNativeLibrary("libvgpu.dylib");
        }
        else
        {
            s_vgpuModule = _loader.LoadNativeLibrary("libvgpu.so");
        }

        if (s_vgpuModule == IntPtr.Zero)
        {
            throw new NotSupportedException("WebGPU is not supported");
        }

        GenLoadCommands();
    }

    private static IntPtr LoadFunctionPointer(string name)
    {
        return _loader.LoadFunctionPointer(s_vgpuModule, name);
    }

    private static ILibraryLoader GetPlatformLoader()
    {
        if (OperatingSystem.IsWindows())
        {
            return new Win32LibraryLoader();
        }

        if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX) ||
            OperatingSystem.IsFreeBSD())
        {
            return new BsdLibraryLoader();
        }

        if (OperatingSystem.IsLinux())
        {
            return new UnixLibraryLoader();
        }

        throw new PlatformNotSupportedException("This platform cannot load native libraries.");
    }

    interface ILibraryLoader
    {
        nint LoadNativeLibrary(string name);
        void FreeNativeLibrary(nint handle);

        nint LoadFunctionPointer(nint handle, string name);
    }

    private class Win32LibraryLoader : ILibraryLoader
    {
        public nint LoadNativeLibrary(string name)
        {
            return LoadLibrary(name);
        }

        public void FreeNativeLibrary(nint handle)
        {
            FreeLibrary(handle);
        }

        public nint LoadFunctionPointer(nint handle, string name)
        {
            return GetProcAddress(handle, name);
        }

        [DllImport("kernel32")]
        private static extern nint LoadLibrary(string fileName);

        [DllImport("kernel32")]
        private static extern int FreeLibrary(nint module);

        [DllImport("kernel32")]
        private static extern nint GetProcAddress(nint module, string procName);
    }

    private class UnixLibraryLoader : ILibraryLoader
    {
        public nint LoadNativeLibrary(string name)
        {
            return Libdl.dlopen(name, Libdl.RTLD_NOW | Libdl.RTLD_LOCAL);
        }

        public void FreeNativeLibrary(nint handle)
        {
            Libdl.dlclose(handle);
        }

        public nint LoadFunctionPointer(nint handle, string name)
        {
            return Libdl.dlsym(handle, name);
        }
    }

    private class BsdLibraryLoader : ILibraryLoader
    {
        public nint LoadNativeLibrary(string name)
        {
            return Libc.dlopen(name, Libc.RTLD_NOW | Libc.RTLD_LOCAL);
        }

        public void FreeNativeLibrary(nint handle)
        {
            Libc.dlclose(handle);
        }

        public nint LoadFunctionPointer(nint handle, string name)
        {
            return Libc.dlsym(handle, name);
        }
    }

    internal static class Libdl
    {
        private const string LibName = "libdl";

        public const int RTLD_LOCAL = 0x000;
        public const int RTLD_NOW = 0x002;

        [DllImport(LibName)]
        public static extern nint dlopen(string fileName, int flags);

        [DllImport(LibName)]
        public static extern nint dlsym(nint handle, string name);

        [DllImport(LibName)]
        public static extern int dlclose(nint handle);

        [DllImport(LibName)]
        public static extern string dlerror();
    }

    internal static class Libc
    {
        private const string LibName = "libc";

        public const int RTLD_LOCAL = 0x000;
        public const int RTLD_NOW = 0x002;

        [DllImport(LibName)]
        public static extern nint dlopen(string fileName, int flags);

        [DllImport(LibName)]
        public static extern nint dlsym(nint handle, string name);

        [DllImport(LibName)]
        public static extern int dlclose(nint handle);

        [DllImport(LibName)]
        public static extern string dlerror();
    }
}

