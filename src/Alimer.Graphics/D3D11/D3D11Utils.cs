// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Win32;
using Win32.Graphics.Direct3D;
using Win32.Graphics.Direct3D11;
using static Win32.Graphics.Direct3D11.Apis;
using D3D11QueryType = Win32.Graphics.Direct3D11.QueryType;

namespace Alimer.Graphics.D3D11;

internal static unsafe class D3D11Utils
{
    public static bool SdkLayersAvailable()
    {
        HResult hr = D3D11CreateDevice(
            null,
            DriverType.Null,       // There is no need to create a real hardware device.
            IntPtr.Zero,
            CreateDeviceFlags.Debug,  // Check for the SDK layers.
            null,                    // Any feature level will do.
            0,
            D3D11_SDK_VERSION,
            null,                    // No need to keep the D3D device reference.
            null,                    // No need to know the feature level.
            null                     // No need to keep the D3D device context reference.
        );

        return hr.Success;
    }

    public static uint BufferSizeAlignment(BufferUsage usage)
    {
        if ((usage & BufferUsage.Constant) != 0)
        {
            // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
            // Each number of constants must be a multiple of 16 shader constants(sizeof(float) * 4 *
            // 16).
            return sizeof(float) * 4 * 16;
        }

        if ((usage & BufferUsage.ShaderWrite) != 0)
        {
            // Unordered access buffers must be 4-byte aligned.
            return sizeof(uint);
        }

        return 1;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D11QueryType ToD3D11(this QueryType value)
    {
        switch (value)
        {
            default:
            case QueryType.Timestamp:
                return D3D11QueryType.Timestamp;

            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
                return D3D11QueryType.Occlusion;

            case QueryType.PipelineStatistics:
                return D3D11QueryType.PipelineStatistics;
        }
    }
}
