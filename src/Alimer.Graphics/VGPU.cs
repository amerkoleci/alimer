// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    public enum GPULogLevel
    {
        Error = 0,
        Warn = 1,
        Info = 2,
        Debug = 3
    }

    public static unsafe class VGPU
    {
        private static readonly IntPtr s_vgpuLibrary;

        public const int MaxAdapterNameSize = 256;

        static VGPU()
        {
            NativeLibrary.SetDllImportResolver(typeof(VGPU).Assembly, ImportResolver);

            //if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            //{
            //    s_vgpuLibrary = NativeLibrary.Load("vgpu.dll");
            //}
            //else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            //{
            //    s_vgpuLibrary = NativeLibrary.Load("vgpu.so");
            //}
            //else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            //{
            //    s_vgpuLibrary = NativeLibrary.Load("vgpu.dylib");
            //}
            //else
            //{
            //    s_vgpuLibrary = NativeLibrary.Load("vgpu");
            //}

            //s_vgpu_set_log_callback_function = LoadFunction<vgpu_set_log_callback_t>(nameof(vgpuSetLogCallback));
            //s_vgpu_device_create = LoadFunction<vgpu_device_create_t>(nameof(vgpuCreateDevice));
            //vgpuDestroyDevice_ptr = LoadFunction<vgpuDestroyDeviceDelegate>(nameof(vgpuDestroyDevice));
            //vgpuBeginFrame_ptr = LoadFunction<vgpuBeginFrameDelegate>(nameof(vgpuBeginFrame));
            //vgpuEndFrame_ptr = LoadFunction<vgpuEndFrameDelegate>(nameof(vgpuEndFrame));
            //vgpuGetDeviceCaps_ptr = LoadFunction<vgpuGetDeviceCapsDelegate>(nameof(vgpuGetDeviceCaps));
            //vgpuGetBackbufferTexture_ptr = LoadFunction<vgpuGetBackbufferTextureDelegate>(nameof(vgpuGetBackbufferTexture));
            //vgpuCmdBeginRenderPass_ptr = LoadFunction<vgpuCmdBeginRenderPassDelegate>(nameof(vgpuCmdBeginRenderPass));
            //vgpuCmdEndRenderPass_ptr = LoadFunction<vgpuCmdEndRenderPassDelegate>(nameof(vgpuCmdEndRenderPass));
        }


        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            string assemblyLocation = Path.GetDirectoryName(assembly.Location);

            IntPtr lib = IntPtr.Zero;
            // Try .NET Framework / mono locations
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                libraryName += ".dylib";
                // Look in Frameworks for .app bundles
                if (!NativeLibrary.TryLoad(Path.Combine(assemblyLocation, libraryName), out lib))
                {
                    NativeLibrary.TryLoad(Path.Combine(assemblyLocation, "..", "Frameworks", libraryName), out lib);
                }
            }
            else
            {
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    libraryName += ".dll";
                }
                else
                {
                    libraryName += ".so";
                }

                if (Environment.Is64BitProcess)
                    NativeLibrary.TryLoad(Path.Combine(assemblyLocation, "x64", libraryName), out lib);
                else
                    NativeLibrary.TryLoad(Path.Combine(assemblyLocation, "x86", libraryName), out lib);
            }

            // Try .NET Core development locations
            if (lib == IntPtr.Zero)
                NativeLibrary.TryLoad(Path.Combine(assemblyLocation, "native", GetPlaformRid(), libraryName), out lib);

            if (lib == IntPtr.Zero)
                NativeLibrary.TryLoad(Path.Combine(assemblyLocation, "runtimes", GetPlaformRid(), "native", libraryName), out lib);

            // Welp, all failed, PANIC!!!
            if (lib == IntPtr.Zero)
                throw new PlatformNotSupportedException("Failed to load library: " + libraryName);

            return lib;
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

        #region Delegates
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void LogCallback(IntPtr userData, GPULogLevel level, string msg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpu_set_log_callback_t(LogCallback callback, IntPtr userData);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr vgpu_device_create_t(BackendType backendType, GPUDeviceInfo* info);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpuDestroyDeviceDelegate(GPUDevice device);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate Bool32 vgpuBeginFrameDelegate(GPUDevice device);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpuEndFrameDelegate(GPUDevice device);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpuGetDeviceCapsDelegate(GPUDevice device, out GPUDeviceCaps caps);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate GPUTexture vgpuGetBackbufferTextureDelegate(GPUDevice device);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpuCmdBeginRenderPassDelegate(GPUDevice device, RenderPassDescription* pass);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void vgpuCmdEndRenderPassDelegate(GPUDevice device);
        #endregion

        [DllImport("vgpu")]
        public static extern void vgpuSetLogCallback(LogCallback callback, IntPtr userData);

        [DllImport("vgpu")]
        public static extern GPUDevice vgpuCreateDevice(BackendType backendType, GPUDeviceInfo info);

        [DllImport("vgpu")]
        public static extern void vgpuDestroyDevice(GPUDevice device);

        [DllImport("vgpu")]
        public static extern bool vgpuBeginFrame(GPUDevice device);

        [DllImport("vgpu")]
        public static extern void vgpuEndFrame(GPUDevice device);

        [DllImport("vgpu")]
        public static extern void vgpuGetDeviceCaps(GPUDevice device, out GPUDeviceCaps caps);

        [DllImport("vgpu")]
        public static extern GPUTexture vgpuGetBackbufferTexture(GPUDevice device);

        [DllImport("vgpu")]
        public static extern void vgpuCmdBeginRenderPass(GPUDevice device, RenderPassDescription pass);

        [DllImport("vgpu")]
        public static extern void vgpuCmdEndRenderPass(GPUDevice device);
    }
}
