// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMPARISON_FUNC;
using static TerraFX.Interop.DirectX.D3D12_FILTER_REDUCTION_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FILTER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_ADDRESS_MODE;
using static TerraFX.Interop.DirectX.D3D12_FILL_MODE;
using static TerraFX.Interop.DirectX.D3D12_CULL_MODE;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_QUERY_HEAP_TYPE;
using static TerraFX.Interop.DirectX.D3D12_QUERY_TYPE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using static TerraFX.Interop.DirectX.D3D12_STATIC_BORDER_COLOR;

namespace Alimer.Graphics.D3D12;

internal static unsafe class D3D12Utils
{
    public static D3D12_HEAP_PROPERTIES DefaultHeapProps => new(D3D12_HEAP_TYPE_DEFAULT);
    public static D3D12_HEAP_PROPERTIES UploadHeapProps => new(D3D12_HEAP_TYPE_UPLOAD);
    public static D3D12_HEAP_PROPERTIES ReadbackHeapProps => new(D3D12_HEAP_TYPE_READBACK);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMMAND_LIST_TYPE ToD3D12(this QueueType queue)
    {
        return queue switch
        {
            QueueType.Compute => D3D12_COMMAND_LIST_TYPE_COMPUTE,
            QueueType.Copy => D3D12_COMMAND_LIST_TYPE_COPY,
            QueueType.VideoDecode => D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
            QueueType.VideoEncode => D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
            _ => D3D12_COMMAND_LIST_TYPE_DIRECT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_RESOURCE_DIMENSION ToD3D12(this TextureDimension value)
    {
        return value switch
        {
            TextureDimension.Texture1D => D3D12_RESOURCE_DIMENSION_TEXTURE1D,
            TextureDimension.Texture2D => D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            TextureDimension.Texture3D => D3D12_RESOURCE_DIMENSION_TEXTURE3D,
            _ => D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMPARISON_FUNC ToD3D12(this CompareFunction function)
    {
        return function switch
        {
            CompareFunction.Never => D3D12_COMPARISON_FUNC_NEVER,
            CompareFunction.Less => D3D12_COMPARISON_FUNC_LESS,
            CompareFunction.Equal => D3D12_COMPARISON_FUNC_EQUAL,
            CompareFunction.LessEqual => D3D12_COMPARISON_FUNC_LESS_EQUAL,
            CompareFunction.Greater => D3D12_COMPARISON_FUNC_GREATER,
            CompareFunction.NotEqual => D3D12_COMPARISON_FUNC_NOT_EQUAL,
            CompareFunction.GreaterEqual => D3D12_COMPARISON_FUNC_GREATER_EQUAL,
            CompareFunction.Always => D3D12_COMPARISON_FUNC_ALWAYS,
            _ => D3D12_COMPARISON_FUNC_NEVER,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_REDUCTION_TYPE ToD3D12(this SamplerReductionType value)
    {
        return value switch
        {
            SamplerReductionType.Standard => D3D12_FILTER_REDUCTION_TYPE_STANDARD,
            SamplerReductionType.Comparison => D3D12_FILTER_REDUCTION_TYPE_COMPARISON,
            SamplerReductionType.Minimum => D3D12_FILTER_REDUCTION_TYPE_MINIMUM,
            SamplerReductionType.Maximum => D3D12_FILTER_REDUCTION_TYPE_MAXIMUM,
            _ => D3D12_FILTER_REDUCTION_TYPE_STANDARD,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_TYPE ToD3D12(this SamplerMinMagFilter filter)
    {
        return filter switch
        {
            SamplerMinMagFilter.Nearest => D3D12_FILTER_TYPE_POINT,
            SamplerMinMagFilter.Linear => D3D12_FILTER_TYPE_LINEAR,
            _ => D3D12_FILTER_TYPE_POINT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILTER_TYPE ToD3D12(this SamplerMipFilter filter)
    {
        return filter switch
        {
            SamplerMipFilter.Nearest => D3D12_FILTER_TYPE_POINT,
            SamplerMipFilter.Linear => D3D12_FILTER_TYPE_LINEAR,
            _ => D3D12_FILTER_TYPE_POINT,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_TEXTURE_ADDRESS_MODE ToD3D12(this SamplerAddressMode filter)
    {
        switch (filter)
        {
            case SamplerAddressMode.Repeat:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddressMode.MirrorRepeat:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddressMode.ClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddressMode.ClampToBorder:
                return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case SamplerAddressMode.MirrorClampToEdge:
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

            default:
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_QUERY_HEAP_TYPE ToD3D12(this QueryType value)
    {
        return value switch
        {
            QueryType.Timestamp => D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
            QueryType.PipelineStatistics => D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS,
            _ => D3D12_QUERY_HEAP_TYPE_OCCLUSION,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_QUERY_TYPE ToD3D12QueryType(this QueryType value)
    {
        return value switch
        {
            QueryType.Occlusion => D3D12_QUERY_TYPE_OCCLUSION,
            QueryType.BinaryOcclusion => D3D12_QUERY_TYPE_BINARY_OCCLUSION,
            QueryType.Timestamp => D3D12_QUERY_TYPE_TIMESTAMP,
            QueryType.PipelineStatistics => D3D12_QUERY_TYPE_PIPELINE_STATISTICS,
            _ => D3D12_QUERY_TYPE_OCCLUSION,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int GetQueryResultSize(this QueryType type)
    {
        switch (type)
        {
            case QueryType.Occlusion:
            case QueryType.BinaryOcclusion:
            case QueryType.Timestamp:
                return sizeof(ulong);

            case QueryType.PipelineStatistics:
                return sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);

            default:
                return 0;
        }
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_FILL_MODE ToD3D12(this FillMode value)
    {
        return value switch
        {
            FillMode.Wireframe => D3D12_FILL_MODE_WIREFRAME,
            _ => D3D12_FILL_MODE_SOLID,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_CULL_MODE ToD3D12(this CullMode value)
    {
        return value switch
        {
            CullMode.Front => D3D12_CULL_MODE_FRONT,
            CullMode.None => D3D12_CULL_MODE_NONE,
            _ => D3D12_CULL_MODE_BACK,
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_SHADING_RATE ToD3D12(this ShadingRate value)
    {
        switch (value)
        {
            case ShadingRate.Rate1x2:
                return D3D12_SHADING_RATE_1X1;
            case ShadingRate.Rate2x1:
                return D3D12_SHADING_RATE_2X1;
            case ShadingRate.Rate2x2:
                return D3D12_SHADING_RATE_2X2;
            case ShadingRate.Rate2x4:
                return D3D12_SHADING_RATE_2X4;
            case ShadingRate.Rate4x2:
                return D3D12_SHADING_RATE_4X2;
            case ShadingRate.Rate4x4:
                return D3D12_SHADING_RATE_4X4;
            default:
                return D3D12_SHADING_RATE_1X1;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_SHADER_VISIBILITY ToD3D12(this ShaderStages stage)
    {
        if (stage == ShaderStages.All)
            return D3D12_SHADER_VISIBILITY_ALL;

        switch (stage)
        {
            case ShaderStages.Vertex:
                return D3D12_SHADER_VISIBILITY_VERTEX;
            case ShaderStages.Hull:
                return D3D12_SHADER_VISIBILITY_HULL;
            case ShaderStages.Domain:
                return D3D12_SHADER_VISIBILITY_DOMAIN;
            case ShaderStages.Geometry:
                return D3D12_SHADER_VISIBILITY_GEOMETRY;
            case ShaderStages.Fragment:
                return D3D12_SHADER_VISIBILITY_PIXEL;
            case ShaderStages.Amplification:
                return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
            case ShaderStages.Mesh:
                return D3D12_SHADER_VISIBILITY_MESH;

            default:
                return D3D12_SHADER_VISIBILITY_ALL;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_DESCRIPTOR_RANGE_TYPE ToD3D12(this BindingInfoType value)
    {
        switch (value)
        {
            case BindingInfoType.Buffer:
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

            case BindingInfoType.Sampler:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

            case BindingInfoType.Texture:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

            case BindingInfoType.StorageTexture:
                return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

            default:
                return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

    public static D3D12_SAMPLER_DESC ToD3D12SamplerDesc(in SamplerDescription description)
    {
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        D3D12_SAMPLER_DESC desc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        desc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), 16u);
        if (desc.MaxAnisotropy > 1)
        {
            desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
        }
        else
        {
            desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reductionType);
        }

        desc.AddressU = description.AddressModeU.ToD3D12();
        desc.AddressV = description.AddressModeV.ToD3D12();
        desc.AddressW = description.AddressModeW.ToD3D12();
        desc.MipLODBias = 0.0f;
        if (description.CompareFunction != CompareFunction.Never)
        {
            desc.ComparisonFunc = description.CompareFunction.ToD3D12();
        }
        else
        {
            desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }
        desc.MinLOD = description.LodMinClamp;
        desc.MaxLOD = description.LodMaxClamp;
        switch (description.BorderColor)
        {
            case SamplerBorderColor.FloatOpaqueBlack:
            case SamplerBorderColor.UintOpaqueBlack:
                desc.BorderColor[0] = 0.0f;
                desc.BorderColor[1] = 0.0f;
                desc.BorderColor[2] = 0.0f;
                desc.BorderColor[3] = 1.0f;
                break;

            case SamplerBorderColor.FloatOpaqueWhite:
            case SamplerBorderColor.UintOpaqueWhite:
                desc.BorderColor[0] = 1.0f;
                desc.BorderColor[1] = 1.0f;
                desc.BorderColor[2] = 1.0f;
                desc.BorderColor[3] = 1.0f;
                break;

            default:
                desc.BorderColor[0] = 0.0f;
                desc.BorderColor[1] = 0.0f;
                desc.BorderColor[2] = 0.0f;
                desc.BorderColor[3] = 0.0f;
                break;
        }

        return desc;
    }

    public static D3D12_STATIC_SAMPLER_DESC ToD3D12StaticSamplerDesc(
        uint shaderRegister,
        in SamplerDescription description,
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, uint registerSpace = 0u)
    {
        D3D12_SAMPLER_DESC samplerDesc = ToD3D12SamplerDesc(in description);

        D3D12_STATIC_SAMPLER_DESC staticDesc = new()
        {
            Filter = samplerDesc.Filter,
            AddressU = samplerDesc.AddressU,
            AddressV = samplerDesc.AddressV,
            AddressW = samplerDesc.AddressW,
            MipLODBias = samplerDesc.MipLODBias,
            MaxAnisotropy = samplerDesc.MaxAnisotropy,
            ComparisonFunc = samplerDesc.ComparisonFunc,
            MinLOD = samplerDesc.MinLOD,
            MaxLOD = samplerDesc.MaxLOD,
            ShaderRegister = shaderRegister,
            RegisterSpace = registerSpace,
            ShaderVisibility = shaderVisibility
        };

        staticDesc.BorderColor = description.BorderColor switch
        {
            SamplerBorderColor.FloatOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
            SamplerBorderColor.UintOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT,
            SamplerBorderColor.FloatOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
            SamplerBorderColor.UintOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT,
            _ => D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
        };

        return staticDesc;
    }

    public static D3D12_RESOURCE_STATES ToD3D12(this ResourceStates states)
    {
        if (states == ResourceStates.Common || states == ResourceStates.Present)
            return D3D12_RESOURCE_STATE_COMMON;

        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON; // also 0

        if ((states & ResourceStates.ConstantBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        if ((states & ResourceStates.VertexBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        if ((states & ResourceStates.IndexBuffer) != 0)
        {
            result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }

        if ((states & ResourceStates.IndirectArgument) != 0)
        {
            result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        }

        if ((states & ResourceStates.ShaderResource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.UnorderedAccess) != 0)
        {
            result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }

        if ((states & ResourceStates.RenderTarget) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        }

        if ((states & ResourceStates.DepthWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }

        if ((states & ResourceStates.DepthRead) != 0)
        {
            result |= D3D12_RESOURCE_STATE_DEPTH_READ;
        }

        if ((states & ResourceStates.StreamOut) != 0)
        {
            result |= D3D12_RESOURCE_STATE_STREAM_OUT;
        }

        if ((states & ResourceStates.CopyDest) != 0)
        {
            result |= D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if ((states & ResourceStates.CopySource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        }

        if ((states & ResourceStates.ResolveDest) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        }

        if ((states & ResourceStates.ResolveSource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        }

        if ((states & ResourceStates.AccelStructRead) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.AccelStructWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.AccelStructBuildInput) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.AccelStructBuildBlas) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        if ((states & ResourceStates.ShadingRateSurface) != 0)
        {
            result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
        }

        if ((states & ResourceStates.OpacityMicromapBuildInput) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        }

        if ((states & ResourceStates.OpacityMicromapWrite) != 0)
        {
            result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        return result;
    }
}
