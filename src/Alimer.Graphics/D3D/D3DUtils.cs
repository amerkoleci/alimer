// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Win32;
using Win32.Graphics.Dxgi.Common;

namespace Alimer.Graphics.D3D;

internal static unsafe class D3DUtils
{
    public const int GENERIC_ALL = (0x10000000);

    public static Format ToDxgiFormat(this VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat.UByte2: return Format.R8G8Uint;
            case VertexFormat.UByte4: return Format.R8G8B8A8Uint;
            case VertexFormat.Byte2: return Format.R8G8Sint;
            case VertexFormat.Byte4: return Format.R8G8B8A8Sint;
            case VertexFormat.UByte2Normalized: return Format.R8G8Unorm;
            case VertexFormat.UByte4Normalized: return Format.R8G8B8A8Unorm;
            case VertexFormat.Byte2Normalized: return Format.R8G8Snorm;
            case VertexFormat.Byte4Normalized: return Format.R8G8B8A8Snorm;

            case VertexFormat.UShort2: return Format.R16G16Uint;
            case VertexFormat.UShort4: return Format.R16G16B16A16Uint;
            case VertexFormat.Short2: return Format.R16G16Sint;
            case VertexFormat.Short4: return Format.R16G16B16A16Sint;
            case VertexFormat.UShort2Normalized: return Format.R16G16Unorm;
            case VertexFormat.UShort4Normalized: return Format.R16G16B16A16Unorm;
            case VertexFormat.Short2Normalized: return Format.R16G16Snorm;
            case VertexFormat.Short4Normalized: return Format.R16G16B16A16Snorm;
            case VertexFormat.Half2: return Format.R16G16Float;
            case VertexFormat.Half4: return Format.R16G16B16A16Float;

            case VertexFormat.Float: return Format.R32Float;
            case VertexFormat.Float2: return Format.R32G32Float;
            case VertexFormat.Float3: return Format.R32G32B32Float;
            case VertexFormat.Float4: return Format.R32G32B32A32Float;

            case VertexFormat.UInt: return Format.R32Uint;
            case VertexFormat.UInt2: return Format.R32G32Uint;
            case VertexFormat.UInt3: return Format.R32G32B32Uint;
            case VertexFormat.UInt4: return Format.R32G32B32A32Uint;

            case VertexFormat.Int: return Format.R32Sint;
            case VertexFormat.Int2: return Format.R32G32Sint;
            case VertexFormat.Int3: return Format.R32G32B32Sint;
            case VertexFormat.Int4: return Format.R32G32B32A32Sint;

            case VertexFormat.Int1010102Normalized: return Format.R10G10B10A2Unorm;
            case VertexFormat.UInt1010102Normalized: return Format.R10G10B10A2Uint;

            default:
                return Format.Unknown;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Format ToDxgiSwapChainFormat(this PixelFormat format)
    {
        // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
        switch (format)
        {
            case PixelFormat.Rgba16Float:
                return Format.R16G16B16A16Float;

            case PixelFormat.Bgra8Unorm:
            case PixelFormat.Bgra8UnormSrgb:
                return Format.B8G8R8A8Unorm;

            case PixelFormat.Rgba8Unorm:
            case PixelFormat.Rgba8UnormSrgb:
                return Format.R8G8B8A8Unorm;

            case PixelFormat.Rgb10a2Unorm:
                return Format.R10G10B10A2Unorm;

            default:
                return Format.B8G8R8A8Unorm;
        }
    }

    public static Format ToDxgiFormat(this IndexType indexType)
    {
        switch (indexType)
        {
            case IndexType.UInt16: return Format.R16Uint;
            case IndexType.UInt32: return Format.R32Uint;

            default:
                return Format.R16Uint;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint ToSampleCount(this TextureSampleCount sampleCount)
    {
        return sampleCount switch
        {
            TextureSampleCount.Count1 => 1,
            TextureSampleCount.Count2 => 2,
            TextureSampleCount.Count4 => 4,
            TextureSampleCount.Count8 => 8,
            TextureSampleCount.Count16 => 16,
            TextureSampleCount.Count32 => 32,
            _ => 1,
        };
    }

    public static uint PresentModeToBufferCount(PresentMode mode)
    {
        switch (mode)
        {
            case PresentMode.Immediate:
            case PresentMode.Fifo:
                return 2;
            case PresentMode.Mailbox:
                return 3;
            default:
                return 2;
        }
    }

    public static uint PresentModeToSyncInterval(PresentMode mode)
    {
        switch (mode)
        {
            case PresentMode.Immediate:
            case PresentMode.Mailbox:
                return 0u;

            case PresentMode.Fifo:
            default:
                return 1u;
        }
    }

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    public static extern int CloseHandle(Handle hObject);
}
