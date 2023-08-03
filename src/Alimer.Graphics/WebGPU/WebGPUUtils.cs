// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal static unsafe class WebGPUUtils
{
    private static readonly WGPUTextureDimension[] s_wgpuTextureTypeMap = new WGPUTextureDimension[(int)TextureDimension.Count] {
        WGPUTextureDimension._1D,
        WGPUTextureDimension._2D,
        WGPUTextureDimension._3D,
    };

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUTextureFormat ToWebGPU(this PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
            case PixelFormat.R8Unorm: return WGPUTextureFormat.R8Unorm;
            case PixelFormat.R8Snorm: return WGPUTextureFormat.R8Snorm;
            case PixelFormat.R8Uint: return WGPUTextureFormat.R8Uint;
            case PixelFormat.R8Sint: return WGPUTextureFormat.R8Sint;
            // 16-bit formats
            //case PixelFormat.R16Unorm: return WGPUTextureFormat.R16Unorm;
            //case PixelFormat.R16Snorm: return WGPUTextureFormat.R16Snorm;
            case PixelFormat.R16Uint: return WGPUTextureFormat.R16Uint;
            case PixelFormat.R16Sint: return WGPUTextureFormat.R16Sint;
            case PixelFormat.R16Float: return WGPUTextureFormat.R16Float;
            case PixelFormat.RG8Unorm: return WGPUTextureFormat.RG8Unorm;
            case PixelFormat.RG8Snorm: return WGPUTextureFormat.RG8Snorm;
            case PixelFormat.RG8Uint: return WGPUTextureFormat.RG8Uint;
            case PixelFormat.RG8Sint: return WGPUTextureFormat.RG8Sint;
            // Packed 16-Bit Pixel Formats
            //case PixelFormat.Bgra4Unorm: return WGPUTextureFormat.Bgra4Unorm;
            //case PixelFormat.B5G6R5Unorm: return WGPUTextureFormat.B5G6R5UnormPack16;
            //case PixelFormat.Bgr5A1Unorm: return WGPUTextureFormat.B5G5R5A1UnormPack16;
            // 32-bit formats
            case PixelFormat.R32Uint: return WGPUTextureFormat.R32Uint;
            case PixelFormat.R32Sint: return WGPUTextureFormat.R32Sint;
            case PixelFormat.R32Float: return WGPUTextureFormat.R32Float;
            //case PixelFormat.Rg16Unorm: return WGPUTextureFormat.Rg16Unorm;
            //case PixelFormat.Rg16Snorm: return WGPUTextureFormat.Rg16Snorm;
            case PixelFormat.RG16Uint: return WGPUTextureFormat.RG16Uint;
            case PixelFormat.RG16Sint: return WGPUTextureFormat.RG16Sint;
            case PixelFormat.RG16Float: return WGPUTextureFormat.RG16Float;
            case PixelFormat.RGBA8Unorm: return WGPUTextureFormat.RGBA8Unorm;
            case PixelFormat.RGBA8UnormSrgb: return WGPUTextureFormat.RGBA8UnormSrgb;
            case PixelFormat.RGBA8Snorm: return WGPUTextureFormat.RGBA8Snorm;
            case PixelFormat.RGBA8Uint: return WGPUTextureFormat.RGBA8Uint;
            case PixelFormat.RGBA8Sint: return WGPUTextureFormat.RGBA8Sint;
            case PixelFormat.BGRA8Unorm: return WGPUTextureFormat.BGRA8Unorm;
            case PixelFormat.BGRA8UnormSrgb: return WGPUTextureFormat.BGRA8UnormSrgb;
            // Packed 32-Bit formats
            case PixelFormat.RGB10A2Unorm: return WGPUTextureFormat.RGB10A2Unorm;
            //case PixelFormat.Rgb10a2Uint: return WGPUTextureFormat.RGB10A2Unorm;
            case PixelFormat.RG11B10UFloat: return WGPUTextureFormat.RG11B10Ufloat;
            case PixelFormat.RGB9E5UFloat: return WGPUTextureFormat.RGB9E5Ufloat;
            // 64-Bit formats
            case PixelFormat.RG32Uint: return WGPUTextureFormat.RG32Uint;
            case PixelFormat.RG32Sint: return WGPUTextureFormat.RG32Sint;
            case PixelFormat.RG32Float: return WGPUTextureFormat.RG32Float;
            //case PixelFormat.Rgba16Unorm: return WGPUTextureFormat.Rgba16Unorm;
            //case PixelFormat.Rgba16Snorm: return WGPUTextureFormat.Rgba16Snorm;
            case PixelFormat.RGBA16Uint: return WGPUTextureFormat.RGBA16Uint;
            case PixelFormat.RGBA16Sint: return WGPUTextureFormat.RGBA16Sint;
            case PixelFormat.RGBA16Float: return WGPUTextureFormat.RGBA16Float;
            // 128-Bit formats
            case PixelFormat.RGBA32Uint:
                return WGPUTextureFormat.RGBA32Uint;
            case PixelFormat.RGBA32Sint:
                return WGPUTextureFormat.RGBA32Sint;
            case PixelFormat.RGBA32Float:
                return WGPUTextureFormat.RGBA32Float;

            // Depth-stencil formats
            //case PixelFormat.Stencil8:
            //    return VkFormat.S8Uint;

            case PixelFormat.Depth16Unorm:
                return WGPUTextureFormat.Depth16Unorm;

            case PixelFormat.Depth24UnormStencil8:
                return WGPUTextureFormat.Depth24PlusStencil8;

            case PixelFormat.Depth32Float:
                return WGPUTextureFormat.Depth32Float;

            case PixelFormat.Depth32FloatStencil8:
                return WGPUTextureFormat.Depth32FloatStencil8;

            // Compressed BC formats
            case PixelFormat.BC1RGBAUnorm:
                return WGPUTextureFormat.BC1RGBAUnorm;
            case PixelFormat.BC1RGBAUnormSrgb:
                return WGPUTextureFormat.BC1RGBAUnormSrgb;
            case PixelFormat.BC2RGBAUnorm:
                return WGPUTextureFormat.BC2RGBAUnorm;
            case PixelFormat.BC2RGBAUnormSrgb:
                return WGPUTextureFormat.BC2RGBAUnormSrgb;
            case PixelFormat.BC3RGBAUnorm:
                return WGPUTextureFormat.BC3RGBAUnorm;
            case PixelFormat.BC3RGBAUnormSrgb:
                return WGPUTextureFormat.BC3RGBAUnormSrgb;
            case PixelFormat.BC4RSnorm:
                return WGPUTextureFormat.BC4RSnorm;
            case PixelFormat.BC4RUnorm:
                return WGPUTextureFormat.BC4RUnorm;
            case PixelFormat.BC5RGUnorm:
                return WGPUTextureFormat.BC5RGUnorm;
            case PixelFormat.BC5RGSnorm:
                return WGPUTextureFormat.BC5RGSnorm;
            case PixelFormat.BC6HRGBUfloat:
                return WGPUTextureFormat.BC6HRGBUfloat;
            case PixelFormat.BC6HRGBFloat:
                return WGPUTextureFormat.BC6HRGBFloat;
            case PixelFormat.BC7RGBAUnorm:
                return WGPUTextureFormat.BC7RGBAUnorm;
            case PixelFormat.BC7RGBAUnormSrgb:
                return WGPUTextureFormat.BC7RGBAUnormSrgb;

            // Etc2/Eac compressed formats
            case PixelFormat.ETC2RGB8Unorm:
                return WGPUTextureFormat.ETC2RGB8Unorm;
            case PixelFormat.ETC2RGB8UnormSrgb:
                return WGPUTextureFormat.ETC2RGB8UnormSrgb;
            case PixelFormat.ETC2RGB8A1Unorm:
                return WGPUTextureFormat.ETC2RGB8A1Unorm;
            case PixelFormat.ETC2RGB8A1UnormSrgb:
                return WGPUTextureFormat.ETC2RGB8A1UnormSrgb;
            case PixelFormat.ETC2RGBA8Unorm:
                return WGPUTextureFormat.ETC2RGBA8Unorm;
            case PixelFormat.ETC2RGBA8UnormSrgb:
                return WGPUTextureFormat.ETC2RGBA8UnormSrgb;
            case PixelFormat.EACR11Unorm:
                return WGPUTextureFormat.EACR11Unorm;
            case PixelFormat.EACR11Snorm:
                return WGPUTextureFormat.EACR11Snorm;
            case PixelFormat.EACRG11Unorm:
                return WGPUTextureFormat.EACRG11Unorm;
            case PixelFormat.EACRG11Snorm:
                return WGPUTextureFormat.EACRG11Snorm;

            default:
                return WGPUTextureFormat.Undefined;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUVertexFormat ToWebGPU(this VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat.UByte2: return WGPUVertexFormat.Uint8x2;
            case VertexFormat.UByte4: return WGPUVertexFormat.Uint8x4;
            case VertexFormat.Byte2: return WGPUVertexFormat.Sint8x2;
            case VertexFormat.Byte4: return WGPUVertexFormat.Sint8x4;
            case VertexFormat.UByte2Normalized: return WGPUVertexFormat.Unorm8x2;
            case VertexFormat.UByte4Normalized: return WGPUVertexFormat.Unorm8x4;
            case VertexFormat.Byte2Normalized: return WGPUVertexFormat.Snorm8x2;
            case VertexFormat.Byte4Normalized: return WGPUVertexFormat.Snorm8x4;

            case VertexFormat.UShort2: return WGPUVertexFormat.Uint16x2;
            case VertexFormat.UShort4: return WGPUVertexFormat.Uint16x4;
            case VertexFormat.Short2: return WGPUVertexFormat.Sint16x2;
            case VertexFormat.Short4: return WGPUVertexFormat.Sint16x4;
            case VertexFormat.UShort2Normalized: return WGPUVertexFormat.Unorm16x2;
            case VertexFormat.UShort4Normalized: return WGPUVertexFormat.Unorm16x4;
            case VertexFormat.Short2Normalized: return WGPUVertexFormat.Snorm16x2;
            case VertexFormat.Short4Normalized: return WGPUVertexFormat.Snorm16x4;
            case VertexFormat.Half2: return WGPUVertexFormat.Float16x2;
            case VertexFormat.Half4: return WGPUVertexFormat.Float16x4;

            case VertexFormat.Float: return WGPUVertexFormat.Float32;
            case VertexFormat.Float2: return WGPUVertexFormat.Float32x2;
            case VertexFormat.Float3: return WGPUVertexFormat.Float32x3;
            case VertexFormat.Float4: return WGPUVertexFormat.Float32x4;

            case VertexFormat.UInt: return WGPUVertexFormat.Uint32;
            case VertexFormat.UInt2: return WGPUVertexFormat.Uint32x2;
            case VertexFormat.UInt3: return WGPUVertexFormat.Uint32x3;
            case VertexFormat.UInt4: return WGPUVertexFormat.Uint32x4;

            case VertexFormat.Int: return WGPUVertexFormat.Sint32;
            case VertexFormat.Int2: return WGPUVertexFormat.Sint32x2;
            case VertexFormat.Int3: return WGPUVertexFormat.Sint32x3;
            case VertexFormat.Int4: return WGPUVertexFormat.Sint32x4;

            //case VertexFormat.Int1010102Normalized: return WGPUVertexFormat.A2B10G10R10SnormPack32;
            //case VertexFormat.UInt1010102Normalized: return WGPUVertexFormat.A2B10G10R10UnormPack32;

            default:
                return WGPUVertexFormat.Undefined;
        }
    }

    public static uint ToWebGPU(this TextureSampleCount sampleCount)
    {
        switch (sampleCount)
        {
            case TextureSampleCount.Count2:
                return 2;
            case TextureSampleCount.Count4:
                return 4;
            case TextureSampleCount.Count8:
                return 8;
            case TextureSampleCount.Count16:
                return 16;
            case TextureSampleCount.Count32:
                return 32;
            default:
                return 1;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUTextureDimension ToWebGPU(this TextureDimension value) => s_wgpuTextureTypeMap[(uint)value];

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUTextureAspect GetWGPUTextureAspect(this WGPUTextureFormat format)
    {
        switch (format)
        {
            case WGPUTextureFormat.Stencil8:
                return WGPUTextureAspect.StencilOnly;

            case WGPUTextureFormat.Depth16Unorm:
            case WGPUTextureFormat.Depth32Float:
                return WGPUTextureAspect.DepthOnly;

            default:
                return WGPUTextureAspect.All;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUPresentMode ToWebGPU(this PresentMode value)
    {
        switch (value)
        {
            default:
            case PresentMode.Fifo:
                return WGPUPresentMode.Fifo;

            case PresentMode.Immediate:
                return WGPUPresentMode.Immediate;

            case PresentMode.Mailbox:
                return WGPUPresentMode.Mailbox;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUQueryType ToWebGPU(this QueryType value)
    {
        switch (value)
        {
            default:
            case QueryType.Timestamp:
                return WGPUQueryType.Timestamp;

            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
                return WGPUQueryType.Occlusion;

            case QueryType.PipelineStatistics:
                return WGPUQueryType.PipelineStatistics;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPULoadOp ToWebGPU(this LoadAction value)
    {
        switch (value)
        {
            default:
            case LoadAction.Load:
                return WGPULoadOp.Load;

            case LoadAction.Clear:
                return WGPULoadOp.Clear;

            case LoadAction.Discard:
                return WGPULoadOp.Undefined;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUStoreOp ToWebGPU(this StoreAction value)
    {
        switch (value)
        {
            default:
            case StoreAction.Store:
                return WGPUStoreOp.Store;

            case StoreAction.Discard:
                return WGPUStoreOp.Discard;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUShaderStage ToWebGPU(this ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage.Vertex:
                return WGPUShaderStage.Vertex;
            case ShaderStage.Hull:
            case ShaderStage.Domain:
            case ShaderStage.Geometry:
            case ShaderStage.Amplification:
            case ShaderStage.Mesh:
                throw new GraphicsException($"WebGPU doesn't support {stage} shader stage");
            case ShaderStage.Fragment:
                return WGPUShaderStage.Fragment;
            case ShaderStage.Compute:
                return WGPUShaderStage.Compute;
            default:
                return WGPUShaderStage.Vertex | WGPUShaderStage.Fragment | WGPUShaderStage.Compute;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUPrimitiveTopology ToWebGPU(this PrimitiveTopology type)
    {
        switch (type)
        {
            case PrimitiveTopology.PointList: return WGPUPrimitiveTopology.PointList;
            case PrimitiveTopology.LineList: return WGPUPrimitiveTopology.LineList;
            case PrimitiveTopology.LineStrip: return WGPUPrimitiveTopology.LineStrip;
            case PrimitiveTopology.TriangleStrip: return WGPUPrimitiveTopology.TriangleStrip;
            //case PrimitiveTopology.PatchList: return WGPUPrimitiveTopology.PatchList;

            default:
                return WGPUPrimitiveTopology.TriangleList;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUCompareFunction ToWebGPU(this CompareFunction value)
    {
        switch (value)
        {
            case CompareFunction.Never: return WGPUCompareFunction.Never;
            case CompareFunction.Less: return WGPUCompareFunction.Less;
            case CompareFunction.Equal: return WGPUCompareFunction.Equal;
            case CompareFunction.LessEqual: return WGPUCompareFunction.LessEqual;
            case CompareFunction.Greater: return WGPUCompareFunction.Greater;
            case CompareFunction.NotEqual: return WGPUCompareFunction.NotEqual;
            case CompareFunction.GreaterEqual: return WGPUCompareFunction.GreaterEqual;
            case CompareFunction.Always: return WGPUCompareFunction.Always;
            default:
                return WGPUCompareFunction.Never;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUVertexStepMode ToWebGPU(this VertexStepMode value)
    {
        return value switch
        {
            VertexStepMode.Instance => WGPUVertexStepMode.Instance,
            _ => WGPUVertexStepMode.Vertex,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUCullMode ToWebGPU(this CullMode value)
    {
        return value switch
        {
            CullMode.Front => WGPUCullMode.Front,
            CullMode.None => WGPUCullMode.None,
            _ => WGPUCullMode.Back,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUBlendFactor ToWebGPU(this BlendFactor value)
    {
        switch (value)
        {
            case BlendFactor.Zero:
                return WGPUBlendFactor.Zero;
            case BlendFactor.One:
                return WGPUBlendFactor.One;
            case BlendFactor.SourceColor:
                return WGPUBlendFactor.Src;
            case BlendFactor.OneMinusSourceColor:
                return WGPUBlendFactor.OneMinusSrc;
            case BlendFactor.SourceAlpha:
                return WGPUBlendFactor.SrcAlpha;
            case BlendFactor.OneMinusSourceAlpha:
                return WGPUBlendFactor.OneMinusSrcAlpha;
            case BlendFactor.DestinationColor:
                return WGPUBlendFactor.Dst;
            case BlendFactor.OneMinusDestinationColor:
                return WGPUBlendFactor.OneMinusDst;
            case BlendFactor.DestinationAlpha:
                return WGPUBlendFactor.DstAlpha;
            case BlendFactor.OneMinusDestinationAlpha:
                return WGPUBlendFactor.OneMinusDstAlpha;
            case BlendFactor.SourceAlphaSaturate:
                return WGPUBlendFactor.SrcAlphaSaturated;
            case BlendFactor.BlendColor:
                return WGPUBlendFactor.Constant;
            case BlendFactor.OneMinusBlendColor:
                return WGPUBlendFactor.OneMinusConstant;
            //case BlendFactor.BlendAlpha:
            //    return WGPUBlendFactor.BlendAlpha;
            //case BlendFactor.OneMinusBlendAlpha:
            //    return WGPUBlendFactor.OneMinusConstantAlpha;
            //case BlendFactor.Source1Color:
            //    return WGPUBlendFactor.Src1Color;
            //case BlendFactor.OneMinusSource1Color:
            //    return WGPUBlendFactor.OneMinusSrc1Color;
            //case BlendFactor.Source1Alpha:
            //    return WGPUBlendFactor.Src1Alpha;
            //case BlendFactor.OneMinusSource1Alpha:
            //    return WGPUBlendFactor.OneMinusSrc1Alpha;
            default:
                return WGPUBlendFactor.Zero;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUBlendOperation ToWebGPU(this BlendOperation value)
    {
        switch (value)
        {
            case BlendOperation.Subtract: return WGPUBlendOperation.Subtract;
            case BlendOperation.ReverseSubtract: return WGPUBlendOperation.ReverseSubtract;
            case BlendOperation.Min: return WGPUBlendOperation.Min;
            case BlendOperation.Max: return WGPUBlendOperation.Max;
            default:
                return WGPUBlendOperation.Add;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUColorWriteMask ToWebGPU(this ColorWriteMask value)
    {
        WGPUColorWriteMask result = WGPUColorWriteMask.None;

        if ((value & ColorWriteMask.Red) != 0)
            result |= WGPUColorWriteMask.Red;

        if ((value & ColorWriteMask.Green) != 0)
            result |= WGPUColorWriteMask.Green;

        if ((value & ColorWriteMask.Blue) != 0)
            result |= WGPUColorWriteMask.Blue;

        if ((value & ColorWriteMask.Alpha) != 0)
            result |= WGPUColorWriteMask.Alpha;

        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUFilterMode ToWebGPU(this SamplerMinMagFilter value)
    {
        return value switch
        {
            SamplerMinMagFilter.Nearest => WGPUFilterMode.Nearest,
            SamplerMinMagFilter.Linear => WGPUFilterMode.Linear,
            _ => WGPUFilterMode.Nearest,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUMipmapFilterMode ToWebGPU(this SamplerMipFilter value)
    {
        return value switch
        {
            SamplerMipFilter.Nearest => WGPUMipmapFilterMode.Nearest,
            SamplerMipFilter.Linear => WGPUMipmapFilterMode.Linear,
            _ => WGPUMipmapFilterMode.Nearest,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static WGPUAddressMode ToWebGPU(this SamplerAddressMode value)
    {
        return value switch
        {
            SamplerAddressMode.Wrap => WGPUAddressMode.Repeat,
            SamplerAddressMode.Mirror => WGPUAddressMode.MirrorRepeat,
            SamplerAddressMode.Clamp => WGPUAddressMode.ClampToEdge,
            //SamplerAddressMode.Border => WGPUAddressMode.Border,
            //SamplerAddressMode.MirrorOnce => samplerMirrorClampToEdge ? VkSamplerAddressMode.MirrorClampToEdge : VkSamplerAddressMode.MirroredRepeat,
            _ => WGPUAddressMode.Repeat,
        };
    }
}
