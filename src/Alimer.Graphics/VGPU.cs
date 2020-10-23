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
        private static readonly NativeLibrary s_vgpuLibrary = LoadLibrary();

        public const int MaxAdapterNameSize = 256;

        private static NativeLibrary LoadLibrary()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return new NativeLibrary("vgpu.dll");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return new NativeLibrary("vgpu.so");
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return new NativeLibrary("vgpu.dylib");
            }
            else
            {
                return new NativeLibrary("vgpu");
            }
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
        private static vgpu_set_log_callback_t s_vgpu_set_log_callback_function = s_vgpuLibrary.LoadFunction<vgpu_set_log_callback_t>(nameof(vgpuSetLogCallback));
        private static vgpu_device_create_t s_vgpu_device_create = s_vgpuLibrary.LoadFunction<vgpu_device_create_t>(nameof(vgpuCreateDevice));
        private static vgpuDestroyDeviceDelegate vgpuDestroyDevice_ptr = s_vgpuLibrary.LoadFunction<vgpuDestroyDeviceDelegate>(nameof(vgpuDestroyDevice));
        private static vgpuBeginFrameDelegate vgpuBeginFrame_ptr = s_vgpuLibrary.LoadFunction<vgpuBeginFrameDelegate>(nameof(vgpuBeginFrame));
        private static vgpuEndFrameDelegate vgpuEndFrame_ptr = s_vgpuLibrary.LoadFunction<vgpuEndFrameDelegate>(nameof(vgpuEndFrame));
        private static vgpuGetDeviceCapsDelegate vgpuGetDeviceCaps_ptr = s_vgpuLibrary.LoadFunction<vgpuGetDeviceCapsDelegate>(nameof(vgpuGetDeviceCaps));
        private static vgpuGetBackbufferTextureDelegate vgpuGetBackbufferTexture_ptr = s_vgpuLibrary.LoadFunction<vgpuGetBackbufferTextureDelegate>(nameof(vgpuGetBackbufferTexture));
        private static vgpuCmdBeginRenderPassDelegate vgpuCmdBeginRenderPass_ptr = s_vgpuLibrary.LoadFunction<vgpuCmdBeginRenderPassDelegate>(nameof(vgpuCmdBeginRenderPass));
        private static vgpuCmdEndRenderPassDelegate vgpuCmdEndRenderPass_ptr = s_vgpuLibrary.LoadFunction<vgpuCmdEndRenderPassDelegate>(nameof(vgpuCmdEndRenderPass));
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
