// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
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
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                s_vgpuLibrary = NativeLibrary.Load("vgpu.dll", typeof(VGPU).Assembly, DllImportSearchPath.AssemblyDirectory | DllImportSearchPath.ApplicationDirectory);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                s_vgpuLibrary = NativeLibrary.Load("vgpu.so");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                s_vgpuLibrary = NativeLibrary.Load("vgpu.dylib");
            }
            else
            {
                s_vgpuLibrary = NativeLibrary.Load("vgpu");
            }

            s_vgpu_set_log_callback_function = LoadFunction<vgpu_set_log_callback_t>(nameof(vgpuSetLogCallback));
            s_vgpu_device_create = LoadFunction<vgpu_device_create_t>(nameof(vgpuCreateDevice));
            vgpuDestroyDevice_ptr = LoadFunction<vgpuDestroyDeviceDelegate>(nameof(vgpuDestroyDevice));
            vgpuBeginFrame_ptr = LoadFunction<vgpuBeginFrameDelegate>(nameof(vgpuBeginFrame));
            vgpuEndFrame_ptr = LoadFunction<vgpuEndFrameDelegate>(nameof(vgpuEndFrame));
            vgpuGetDeviceCaps_ptr = LoadFunction<vgpuGetDeviceCapsDelegate>(nameof(vgpuGetDeviceCaps));
            vgpuGetBackbufferTexture_ptr = LoadFunction<vgpuGetBackbufferTextureDelegate>(nameof(vgpuGetBackbufferTexture));
            vgpuCmdBeginRenderPass_ptr = LoadFunction<vgpuCmdBeginRenderPassDelegate>(nameof(vgpuCmdBeginRenderPass));
            vgpuCmdEndRenderPass_ptr = LoadFunction<vgpuCmdEndRenderPassDelegate>(nameof(vgpuCmdEndRenderPass));
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

        #region Function Pointers
        private static T LoadFunction<T>(string name)
        {
            IntPtr handle = NativeLibrary.GetExport(s_vgpuLibrary, name);
            return Marshal.GetDelegateForFunctionPointer<T>(handle);
        }

        private static readonly vgpu_set_log_callback_t s_vgpu_set_log_callback_function;
        private static readonly vgpu_device_create_t s_vgpu_device_create;
        private static readonly vgpuDestroyDeviceDelegate vgpuDestroyDevice_ptr;
        private static readonly vgpuBeginFrameDelegate vgpuBeginFrame_ptr;
        private static readonly vgpuEndFrameDelegate vgpuEndFrame_ptr;
        private static readonly vgpuGetDeviceCapsDelegate vgpuGetDeviceCaps_ptr;
        private static readonly vgpuGetBackbufferTextureDelegate vgpuGetBackbufferTexture_ptr;
        private static readonly vgpuCmdBeginRenderPassDelegate vgpuCmdBeginRenderPass_ptr;
        private static readonly vgpuCmdEndRenderPassDelegate vgpuCmdEndRenderPass_ptr;
        #endregion

        public static void vgpuSetLogCallback(LogCallback callback, IntPtr userData) => s_vgpu_set_log_callback_function(callback, userData);
        public static GPUDevice vgpuCreateDevice(BackendType backendType, GPUDeviceInfo info) => s_vgpu_device_create(backendType, &info);
        public static void vgpuDestroyDevice(GPUDevice device) => vgpuDestroyDevice_ptr(device);
        public static bool vgpuBeginFrame(GPUDevice device) => vgpuBeginFrame_ptr(device);
        public static void vgpuEndFrame(GPUDevice device) => vgpuEndFrame_ptr(device);
        public static void vgpuGetDeviceCaps(GPUDevice device, out GPUDeviceCaps caps) => vgpuGetDeviceCaps_ptr(device, out caps);
        public static GPUTexture vgpuGetBackbufferTexture(GPUDevice device) => vgpuGetBackbufferTexture_ptr(device);

        public static void vgpuCmdBeginRenderPass(GPUDevice device, RenderPassDescription pass) => vgpuCmdBeginRenderPass_ptr(device, &pass);
        public static void vgpuCmdEndRenderPass(GPUDevice device) => vgpuCmdEndRenderPass_ptr(device);
    }
}
