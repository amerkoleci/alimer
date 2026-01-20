// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_ACCESS;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_LAYOUT;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_SYNC;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.DirectX.D3D12_COMPARISON_FUNC;
using static TerraFX.Interop.DirectX.D3D12_CULL_MODE;
using static TerraFX.Interop.DirectX.D3D12_DESCRIPTOR_RANGE_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FILL_MODE;
using static TerraFX.Interop.DirectX.D3D12_FILTER_REDUCTION_TYPE;
using static TerraFX.Interop.DirectX.D3D12_FILTER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_SHADER_VISIBILITY;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_STATIC_BORDER_COLOR;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_ADDRESS_MODE;
using static TerraFX.Interop.DirectX.DirectX;

namespace Alimer.Graphics.D3D12;

internal static unsafe class D3D12Utils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static D3D12_COMMAND_LIST_TYPE ToD3D12(this CommandQueueType queue)
    {
        return queue switch
        {
            CommandQueueType.Compute => D3D12_COMMAND_LIST_TYPE_COMPUTE,
            CommandQueueType.Copy => D3D12_COMMAND_LIST_TYPE_COPY,
            CommandQueueType.VideoDecode => D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
            //CommandQueueType.VideoEncode => D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
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

    public static D3D12TextureLayoutMapping ConvertTextureLayout(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout.Undefined:
                return new(
                    D3D12_BARRIER_LAYOUT_COMMON,
                    D3D12_BARRIER_SYNC_NONE,
                    D3D12_BARRIER_ACCESS_COMMON
                    );

            case TextureLayout.CopySource:
                return new(
                    D3D12_BARRIER_LAYOUT_COPY_SOURCE,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_SOURCE
                    );

            case TextureLayout.CopyDest:
                return new(
                    D3D12_BARRIER_LAYOUT_COPY_DEST,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_DEST
                    );

            case TextureLayout.ResolveSource:
                return new(
                    D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_SOURCE
                    );

            case TextureLayout.ResolveDest:
                return new(
                    D3D12_BARRIER_LAYOUT_RESOLVE_DEST,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_DEST
                    );

            case TextureLayout.ShaderResource:
                return new(
                    D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_SHADER_RESOURCE
                    );

            case TextureLayout.UnorderedAccess:
                return new(
                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
                    );

            case TextureLayout.RenderTarget:
                return new(
                    D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                    D3D12_BARRIER_SYNC_RENDER_TARGET,
                    D3D12_BARRIER_ACCESS_RENDER_TARGET
                    );

            case TextureLayout.DepthWrite:
                return new(
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,
                    D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                    D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE
                    );

            case TextureLayout.DepthRead:
            return new(
                D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ,
                D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ
                );

            case TextureLayout.Present:
            return new(
                D3D12_BARRIER_LAYOUT_PRESENT,
                D3D12_BARRIER_SYNC_ALL,
                D3D12_BARRIER_ACCESS_COMMON
                );

            case TextureLayout.ShadingRateSurface:
            return new(
                D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE,
                D3D12_BARRIER_SYNC_PIXEL_SHADING,
                D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE
                );

            default:

                return ThrowHelper.ThrowArgumentException<D3D12TextureLayoutMapping>();
    }
    }

    public static D3D12_RESOURCE_STATES ConvertTextureLayoutLegacy(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout.Undefined:
                return D3D12_RESOURCE_STATE_COMMON;

            case TextureLayout.CopySource:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;

            case TextureLayout.CopyDest:
                return D3D12_RESOURCE_STATE_COPY_DEST;

            case TextureLayout.ShaderResource:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            case TextureLayout.UnorderedAccess:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            case TextureLayout.RenderTarget:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case TextureLayout.DepthWrite:
                return D3D12_RESOURCE_STATE_DEPTH_WRITE;

            case TextureLayout.DepthRead:
                return D3D12_RESOURCE_STATE_DEPTH_READ;

            case TextureLayout.Present:
                return D3D12_RESOURCE_STATE_PRESENT;

            case TextureLayout.ShadingRateSurface:
                return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

            default:
                return ThrowHelper.ThrowArgumentException<D3D12_RESOURCE_STATES>("Unsupported texture layout");
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

    public static D3D12_SAMPLER_DESC ToD3D12SamplerDesc(in SamplerDescriptor description)
    {
        D3D12_FILTER_TYPE minFilter = description.MinFilter.ToD3D12();
        D3D12_FILTER_TYPE magFilter = description.MagFilter.ToD3D12();
        D3D12_FILTER_TYPE mipmapFilter = description.MipFilter.ToD3D12();

        D3D12_FILTER_REDUCTION_TYPE reductionType = description.ReductionType.ToD3D12();

        D3D12_SAMPLER_DESC desc = new();

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        desc.MaxAnisotropy = Math.Min(Math.Max(description.MaxAnisotropy, 1u), D3D12_DEFAULT_MAX_ANISOTROPY);
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
        in SamplerDescriptor description,
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
            ShaderVisibility = shaderVisibility,
            BorderColor = description.BorderColor switch
            {
                SamplerBorderColor.FloatOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
                SamplerBorderColor.UintOpaqueBlack => D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT,
                SamplerBorderColor.FloatOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
                SamplerBorderColor.UintOpaqueWhite => D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT,
                _ => D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
            }
        };

        return staticDesc;
    }

    public static D3D12_RESOURCE_STATES ConvertBufferStateLegacy(this BufferStates states, CommandQueueType queueType)
    {
        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

        if ((states & BufferStates.CopyDest) != 0)
            result |= D3D12_RESOURCE_STATE_COPY_DEST;

        if ((states & BufferStates.CopySource) != 0)
            result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

        if ((states & BufferStates.ShaderResource) != 0)
        {
            result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if (queueType == CommandQueueType.Graphics)
                result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }

        if ((states & BufferStates.UnorderedAccess) != 0)
            result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        if ((states & BufferStates.VertexBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if ((states & BufferStates.IndexBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if ((states & BufferStates.ConstantBuffer) != 0)
            result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if ((states & BufferStates.Predication) != 0)
            result |= D3D12_RESOURCE_STATE_PREDICATION;
#if TODO
            if ((stateBits & ResourceStates::IndirectArgument) != 0) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            if ((stateBits & ResourceStates::StreamOut) != 0) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
            if ((stateBits & ResourceStates::AccelerationStructureRead) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::ShadingRateSurface) != 0) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
            if ((stateBits & ResourceStates::OpacityMicromapBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if ((stateBits & ResourceStates::OpacityMicromapWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif // TODO


        return result;
    }

    public readonly record struct D3D12TextureLayoutMapping(D3D12_BARRIER_LAYOUT Layout, D3D12_BARRIER_SYNC Sync, D3D12_BARRIER_ACCESS Access);
}
