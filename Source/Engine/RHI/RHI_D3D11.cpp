// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_D3D11)
#include "Window.h"
#include "Core/Log.h"
#include "RHI_D3D11.h"

#if !defined(ALIMER_DISABLE_SHADER_COMPILER)
#include <d3dcompiler.h>
#endif

#include <array>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace alimer::rhi
{
    extern DXGI_FORMAT ToDXGIFormat(Format format);
    
    namespace
    {
#if !defined(ALIMER_DISABLE_SHADER_COMPILER) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        pD3DCompile D3DCompile;
#endif

        // Check for SDK Layer support.
        inline bool SdkLayersAvailable() noexcept
        {
            HRESULT hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
                nullptr,
                D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
                nullptr,                    // Any feature level will do.
                0,
                D3D11_SDK_VERSION,
                nullptr,                    // No need to keep the D3D device reference.
                nullptr,                    // No need to know the feature level.
                nullptr                     // No need to keep the D3D device context reference.
            );

            return SUCCEEDED(hr);
        }

        inline void SetDebugName(ID3D11DeviceChild* pObject, const std::string_view& name)
        {
            D3D_SET_OBJECT_NAME_N_A(pObject, UINT(name.size()), name.data());
        }

        [[nodiscard]] constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(Format format)
        {
            switch (format)
            {
                case Format::Depth16UNorm:
                    return DXGI_FORMAT_R16_TYPELESS;
                case Format::Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;
                case Format::Depth24UNormStencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case Format::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    ALIMER_ASSERT(IsDepthFormat(format) == false);
                    return ToDXGIFormat(format);
            }
        }

        [[nodiscard]] constexpr D3D_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(PrimitiveTopology topology, uint32_t controlPoints)
        {
            switch (topology)
            {
                case PrimitiveTopology::PointList:
                    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case PrimitiveTopology::LineList:
                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case PrimitiveTopology::LineStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case PrimitiveTopology::TriangleList:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case PrimitiveTopology::TriangleStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                case PrimitiveTopology::PatchList:
                    if (controlPoints == 0 || controlPoints > 32)
                    {
                        assert(false && "Invalid PatchList control points");
                        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                    }
                    return D3D_PRIMITIVE_TOPOLOGY(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (controlPoints - 1));
                default:
                    ALIMER_UNREACHABLE();
                    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        [[nodiscard]] constexpr D3D11_COMPARISON_FUNC ToD3D11(CompareFunction function)
        {
            switch (function)
            {
                case CompareFunction::Never:
                    return D3D11_COMPARISON_NEVER;
                case CompareFunction::Less:
                    return D3D11_COMPARISON_LESS;
                case CompareFunction::Equal:
                    return D3D11_COMPARISON_EQUAL;
                case CompareFunction::LessEqual:
                    return D3D11_COMPARISON_LESS_EQUAL;
                case CompareFunction::Greater:
                    return D3D11_COMPARISON_GREATER;
                case CompareFunction::NotEqual:
                    return D3D11_COMPARISON_NOT_EQUAL;
                case CompareFunction::GreaterEqual:
                    return D3D11_COMPARISON_GREATER_EQUAL;
                case CompareFunction::Always:
                    return D3D11_COMPARISON_ALWAYS;

                default:
                    ALIMER_UNREACHABLE();
                    return static_cast<D3D11_COMPARISON_FUNC>(0);
            }
        }

        [[nodiscard]] constexpr D3D11_BLEND D3D11Blend(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::Zero:
                    return D3D11_BLEND_ZERO;
                case BlendFactor::One:
                    return D3D11_BLEND_ONE;
                case BlendFactor::SourceColor:
                    return D3D11_BLEND_SRC_COLOR;
                case BlendFactor::OneMinusSourceColor:
                    return D3D11_BLEND_INV_SRC_COLOR;
                case BlendFactor::SourceAlpha:
                    return D3D11_BLEND_SRC_ALPHA;
                case BlendFactor::OneMinusSourceAlpha:
                    return D3D11_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D11_BLEND_DEST_COLOR;
                case BlendFactor::OneMinusDestinationColor:
                    return D3D11_BLEND_INV_DEST_COLOR;
                case BlendFactor::DestinationAlpha:
                    return D3D11_BLEND_DEST_ALPHA;
                case BlendFactor::OneMinusDestinationAlpha:
                    return D3D11_BLEND_INV_DEST_ALPHA;
                case BlendFactor::SourceAlphaSaturated:
                    return D3D11_BLEND_SRC_ALPHA_SAT;
                case BlendFactor::BlendColor:
                    return D3D11_BLEND_BLEND_FACTOR;
                case BlendFactor::OneMinusBlendColor:
                    return D3D11_BLEND_INV_BLEND_FACTOR;
                case BlendFactor::Source1Color:
                    return D3D11_BLEND_SRC1_COLOR;
                case BlendFactor::OneMinusSource1Color:
                    return D3D11_BLEND_INV_SRC1_COLOR;
                case BlendFactor::Source1Alpha:
                    return D3D11_BLEND_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Alpha:
                    return D3D11_BLEND_INV_SRC1_ALPHA;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr D3D11_BLEND D3D11AlphaBlend(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::SourceColor:
                    return D3D11_BLEND_SRC_ALPHA;
                case BlendFactor::OneMinusSourceColor:
                    return D3D11_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D11_BLEND_DEST_ALPHA;
                case BlendFactor::OneMinusDestinationColor:
                    return D3D11_BLEND_INV_DEST_ALPHA;
                case BlendFactor::SourceAlpha:
                    return D3D11_BLEND_SRC_ALPHA;
                case BlendFactor::Source1Color:
                    return D3D11_BLEND_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Color:
                    return D3D11_BLEND_INV_SRC1_ALPHA;
                    // Other blend factors translate to the same D3D12 enum as the color blend factors.
                default:
                    return D3D11Blend(factor);
            }
        }

        [[nodiscard]] constexpr D3D11_BLEND_OP D3D11BlendOperation(BlendOperation operation)
        {
            switch (operation)
            {
                case BlendOperation::Add:
                    return D3D11_BLEND_OP_ADD;
                case BlendOperation::Subtract:
                    return D3D11_BLEND_OP_SUBTRACT;
                case BlendOperation::ReverseSubtract:
                    return D3D11_BLEND_OP_REV_SUBTRACT;
                case BlendOperation::Min:
                    return D3D11_BLEND_OP_MIN;
                case BlendOperation::Max:
                    return D3D11_BLEND_OP_MAX;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr uint8_t D3D11RenderTargetWriteMask(ColorWriteMask mask)
        {
            static_assert(static_cast<D3D11_COLOR_WRITE_ENABLE>(ColorWriteMask::Red) == D3D11_COLOR_WRITE_ENABLE_RED, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D11_COLOR_WRITE_ENABLE>(ColorWriteMask::Green) == D3D11_COLOR_WRITE_ENABLE_GREEN, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D11_COLOR_WRITE_ENABLE>(ColorWriteMask::Blue) == D3D11_COLOR_WRITE_ENABLE_BLUE, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D11_COLOR_WRITE_ENABLE>(ColorWriteMask::Alpha) == D3D11_COLOR_WRITE_ENABLE_ALPHA, "ColorWriteMask mismatch");
            return static_cast<uint8_t>(mask);
        }

        [[nodiscard]] constexpr D3D11_STENCIL_OP ToD3D11(StencilOperation op)
        {
            switch (op) {
                case StencilOperation::Keep:
                    return D3D11_STENCIL_OP_KEEP;
                case StencilOperation::Zero:
                    return D3D11_STENCIL_OP_ZERO;
                case StencilOperation::Replace:
                    return D3D11_STENCIL_OP_REPLACE;
                case StencilOperation::IncrementClamp:
                    return D3D11_STENCIL_OP_INCR_SAT;
                case StencilOperation::DecrementClamp:
                    return D3D11_STENCIL_OP_DECR_SAT;
                case StencilOperation::Invert:
                    return D3D11_STENCIL_OP_INVERT;
                case StencilOperation::IncrementWrap:
                    return D3D11_STENCIL_OP_INCR;
                case StencilOperation::DecrementWrap:
                    return D3D11_STENCIL_OP_DECR;
                default:
                    ALIMER_UNREACHABLE();
            }
        }
    }

    
    D3D11_Texture::~D3D11_Texture()
    {
        shaderResourceViews.clear();
        renderTargetViews.clear();
        depthStencilViews.clear();
        unorderedAccessViews.clear();

        if (handle)
        {
            handle->Release();
            handle = nullptr;
        }
    }

    IDevice* D3D11_Texture::GetDevice() const
    {
        return device;
    }

    ID3D11RenderTargetView* D3D11_Texture::GetRTV(uint32_t mipLevel, uint32_t slice, uint32_t arraySize)
    {
        if (arraySize == kAllArraySlices)
        {
            arraySize = desc.depthOrArraySize - slice;
        }
        else if (arraySize + slice > desc.depthOrArraySize)
        {
            arraySize = desc.depthOrArraySize - slice;
        }

        D3D11_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), Format::Undefined, false);

        RefCountPtr<ID3D11RenderTargetView>& view = renderTargetViews[key];
        if (view == nullptr)
        {
            D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
            viewDesc.Format = ToDXGIFormat(key.format);

            switch (desc.dimension)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case TextureDimension::Texture1D:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture1DArray:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2D:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture2DArray:
                case TextureDimension::TextureCube:
                case TextureDimension::TextureCubeArray:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = mipLevel;
                    viewDesc.Texture2DArray.FirstArraySlice = slice;
                    viewDesc.Texture2DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2DMS:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                    break;
                case TextureDimension::Texture2DMSArray:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture3D:
                    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
                    viewDesc.Texture3D.MipSlice = mipLevel;
                    viewDesc.Texture3D.FirstWSlice = slice;
                    viewDesc.Texture3D.WSize = arraySize;
                    break;
                default:
                    LOGE("Texture has unsupported dimension for RTV: {}", ToString(desc.dimension));
                    return nullptr;
            }

            HRESULT hr = device->GetD3DDevice()->CreateRenderTargetView(handle, &viewDesc, &view);
            if (FAILED(hr))
            {
                LOGE("Direct3D11: Failed to create RTV");
                return nullptr;
            }
        }

        return view;
    }

    ID3D11DepthStencilView* D3D11_Texture::GetDSV(uint32_t mipLevel, uint32_t slice, uint32_t arraySize, bool isReadOnly)
    {
        if (arraySize == kAllArraySlices)
        {
            arraySize = desc.depthOrArraySize - slice;
        }
        else if (arraySize + slice > desc.depthOrArraySize)
        {
            arraySize = desc.depthOrArraySize - slice;
        }

        D3D11_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), Format::Undefined, isReadOnly);

        RefCountPtr<ID3D11DepthStencilView>& view = depthStencilViews[key];
        if (view == nullptr)
        {
            //we haven't seen this one before
            D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
            viewDesc.Format = ToDXGIFormat(desc.format);
            viewDesc.Flags = 0;

            if (isReadOnly)
            {
                viewDesc.Flags |= D3D11_DSV_READ_ONLY_DEPTH;
                if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
                    viewDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
            }

            switch (desc.dimension)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case TextureDimension::Texture1D:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture1DArray:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2D:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture2DArray:
                case TextureDimension::TextureCube:
                case TextureDimension::TextureCubeArray:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = mipLevel;
                    viewDesc.Texture2DArray.ArraySize = slice;
                    viewDesc.Texture2DArray.FirstArraySlice = arraySize;
                    break;
                case TextureDimension::Texture2DMS:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                    break;
                case TextureDimension::Texture2DMSArray:
                    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    break;
                default:
                    LOGE("Texture has unsupported dimension for DSV: {}", ToString(desc.dimension));
                    return nullptr;
            }

            HRESULT hr = device->GetD3DDevice()->CreateDepthStencilView(handle, &viewDesc, &view);
            if (FAILED(hr))
            {
                LOGE("Direct3D11: Failed to create DSV");
                return nullptr;
            }
        }

        return view;
    }

    void D3D11_Texture::ApiSetName(const std::string_view& newName)
    {
        SetDebugName(handle, newName);
    }

    void D3D11_Shader::ApiSetName(const std::string_view& newName)
    {
        if (VS)
            SetDebugName(VS, newName);

        if (PS)
            SetDebugName(PS, newName);
    }

    void D3D11_Pipeline::ApiSetName(const std::string_view& newName)
    {

    }

    /* D3D11_CommandList */
    D3D11_CommandList::D3D11_CommandList(IDevice* device_, ID3D11DeviceContext* context_)
        : device(device_)
    {
        ThrowIfFailed(context_->QueryInterface(IID_PPV_ARGS(&context)));
        ThrowIfFailed(context_->QueryInterface(IID_PPV_ARGS(&annotation)));
    }

    void D3D11_CommandList::PushDebugGroup(const std::string_view& name)
    {
        if (annotation)
        {
            wchar_t buffer[512];
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.data(), -1, buffer, ARRAYSIZE(buffer));
            annotation->BeginEvent(buffer);
        }
    }

    void D3D11_CommandList::PopDebugGroup()
    {
        if (annotation)
        {
            annotation->EndEvent();
        }
    }

    void D3D11_CommandList::InsertDebugMarker(const std::string_view& name)
    {
        if (annotation)
        {
            wchar_t buffer[512];
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.data(), -1, buffer, ARRAYSIZE(buffer));
            annotation->SetMarker(buffer);
        }
    }

    void D3D11_CommandList::BeginDefaultRenderPass(const Color& clearColor, bool clearDepth, bool clearStencil, float depth, uint8_t stencil)
    {
        RenderPassDesc passDesc;
        passDesc.colorAttachments[0].texture = device->GetCurrentBackBuffer();
        passDesc.colorAttachments[0].loadAction = LoadAction::Clear;
        passDesc.colorAttachments[0].clearColor = clearColor;

        auto depthStencilTexture = device->GetBackBufferDepthStencilTexture();
        if (depthStencilTexture != nullptr)
        {
            passDesc.depthStencilAttachment.texture = depthStencilTexture;
            if (clearDepth)
            {
                passDesc.depthStencilAttachment.depthLoadAction = LoadAction::Clear;
                passDesc.depthStencilAttachment.clearDepth = depth;
            }

            if (clearStencil)
            {
                passDesc.depthStencilAttachment.stencilLoadAction = LoadAction::Clear;
                passDesc.depthStencilAttachment.clearStencil = stencil;
            }
        }

        BeginRenderPass(passDesc);
    }

    void D3D11_CommandList::BeginRenderPass(const RenderPassDesc& desc)
    {
        currentPass = desc;
        uint32_t width = UINT32_MAX;
        uint32_t height = UINT32_MAX;

        uint32_t RTVCount = 0;
        std::array<ID3D11RenderTargetView*, kMaxColorAttachments> RTVs;
        ID3D11DepthStencilView* DSV = nullptr;

        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = desc.colorAttachments[i];
            if (attachment.texture == nullptr)
                break;

            auto d3d11Texture = checked_cast<D3D11_Texture*>(attachment.texture);
            const TextureDesc& textureDesc = d3d11Texture->GetDesc();

            const uint32_t mipLevel = attachment.mipLevel;
            const uint32_t slice = attachment.slice;

            width = Min(width, std::max(1u, textureDesc.width >> mipLevel));
            height = Min(height, std::max(1u, textureDesc.height >> mipLevel));

            RTVs[RTVCount] = d3d11Texture->GetRTV(mipLevel, slice, 1);

            switch (attachment.loadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    context->ClearRenderTargetView(RTVs[RTVCount], &attachment.clearColor.r);
                    break;

                case LoadAction::Discard:
                    context->DiscardView(RTVs[RTVCount]);
                    break;
            }

            RTVCount++;
        }

        if (desc.depthStencilAttachment.texture != nullptr)
        {
            const RenderPassDepthStencilAttachment& attachment = desc.depthStencilAttachment;

            auto d3d11Texture = checked_cast<D3D11_Texture*>(attachment.texture);
            const TextureDesc& textureDesc = d3d11Texture->GetDesc();

            width = Min(width, std::max(1u, textureDesc.width >> attachment.mipLevel));
            height = Min(height, std::max(1u, textureDesc.height >> attachment.mipLevel));

            DSV = d3d11Texture->GetDSV(attachment.mipLevel, attachment.slice, 1, desc.depthStencilAttachment.depthStencilReadOnly);

            UINT clearFlags = 0;

            switch (desc.depthStencilAttachment.depthLoadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    clearFlags |= D3D11_CLEAR_DEPTH;
                    break;

                case LoadAction::Discard:
                    context->DiscardView(DSV);
                    break;
            }

            switch (desc.depthStencilAttachment.stencilLoadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    clearFlags |= D3D11_CLEAR_STENCIL;
                    break;

                case LoadAction::Discard:
                    context->DiscardView(DSV);
                    break;
            }

            if (clearFlags != 0)
            {
                context->ClearDepthStencilView(DSV, clearFlags, desc.depthStencilAttachment.clearDepth, desc.depthStencilAttachment.clearStencil);
            }
        }

        const bool pixelShaderHasUAVs = false;
        if (pixelShaderHasUAVs)
        {
            context->OMSetRenderTargetsAndUnorderedAccessViews(RTVCount, RTVs.data(), DSV,
                D3D11_KEEP_UNORDERED_ACCESS_VIEWS, 0, nullptr, nullptr
            );
        }
        else
        {
            context->OMSetRenderTargets(RTVCount, RTVs.data(), DSV);
        }

        // The viewport and scissor default to cover all of the attachments
        const D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
        const D3D11_RECT scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

        context->RSSetViewports(1, &viewport);
        context->RSSetScissorRects(1, &scissorRect);
    }

    void D3D11_CommandList::EndRenderPass()
    {
        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = currentPass.colorAttachments[i];
            if (attachment.texture == nullptr)
                break;

            auto d3d11Texture = checked_cast<D3D11_Texture*>(attachment.texture);

            switch (attachment.storeAction)
            {
                case StoreAction::Discard:
                {
                    auto RTV = d3d11Texture->GetRTV(attachment.mipLevel, attachment.slice, 1);
                    context->DiscardView(RTV);
                    break;
                }

                //case StoreAction::Resolve:
                //case StoreAction::StoreAndResolve:
                //{
                //    auto resolveTexture = checked_cast<D3D11_Texture*>(attachment.resolveTexture);
                //    uint32_t dstSubresource = D3D11CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, resolveTexture->desc.mipLevels);
                //    uint32_t srcSubresource = D3D11CalcSubresource(attachment.mipLevel, attachment.slice, d3d11Texture->desc.mipLevels);
                //    context->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                //    break;
                //}

                default:
                    break;
            }
        }

        if (currentPass.depthStencilAttachment.texture != nullptr)
        {
            const RenderPassDepthStencilAttachment& attachment = currentPass.depthStencilAttachment;
            auto d3d11Texture = checked_cast<D3D11_Texture*>(attachment.texture);

            switch (attachment.depthStoreAction)
            {
                case StoreAction::Discard:
                {
                    auto DSV = d3d11Texture->GetDSV(attachment.mipLevel, attachment.slice, 1);
                    context->DiscardView(DSV);
                    break;
                }

                //case StoreAction::Resolve:
                //case StoreAction::StoreAndResolve:
                //{
                //    auto resolveTexture = checked_cast<D3D11_Texture*>(attachment.resolveTexture);
                //    uint32_t dstSubresource = D3D11CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, resolveTexture->desc.mipLevels);
                //    uint32_t srcSubresource = D3D11CalcSubresource(attachment.mipLevel, attachment.slice, d3d11Texture->desc.mipLevels);
                //    context->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                //    break;
                //}

                default:
                    break;
            }
        }
    }

    void D3D11_CommandList::SetPipeline(_In_ IPipeline* pipeline)
    {
        D3D11_Pipeline* d3d11Pipeline = checked_cast<D3D11_Pipeline*>(pipeline);
        BindRenderPipeline(d3d11Pipeline);
    }

    void D3D11_CommandList::BindRenderPipeline(const D3D11_Pipeline* pipeline)
    {
        context->VSSetShader(pipeline->vertex, nullptr, 0);
        context->HSSetShader(pipeline->hull, nullptr, 0);
        context->DSSetShader(pipeline->domain, nullptr, 0);
        context->GSSetShader(pipeline->geometry, nullptr, 0);
        context->PSSetShader(pipeline->pixel, nullptr, 0);

        context->IASetPrimitiveTopology(pipeline->primitiveTopology);
        context->IASetInputLayout(pipeline->inputLayout);

        float blendColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetBlendState(pipeline->blendState, blendColor, D3D11_DEFAULT_SAMPLE_MASK);
        context->OMSetDepthStencilState(pipeline->depthStencilState, D3D11_DEFAULT_STENCIL_REFERENCE);
        context->RSSetState(pipeline->rasterizerState);
    }

    void D3D11_CommandList::Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
    {
        if (instanceCount > 1)
        {
            context->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
        }
        else
        {
            context->Draw(vertexCount, vertexStart);
        }
    }

    /* D3D11_Device */
    D3D11_Device::D3D11_Device()
    {
        CreateFactory();

        // Determines whether tearing support is available for fullscreen borderless windows.
        {
            RefCountPtr<IDXGIFactory5> dxgiFactory5;
            if (SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory5))))
            {
                BOOL supported = 0;
                if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
                {
                    tearingSupported = (supported != 0);
                }
            }
        }
    }

    D3D11_Device::~D3D11_Device()
    {
    }

    bool D3D11_Device::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        RefCountPtr<IDXGIAdapter1> adapter;
        GetAdapter(adapter.GetAddressOf());

        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        if (presentationParameters.validationMode != ValidationMode::Disabled)
        {
            if (SdkLayersAvailable())
            {
                // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }
        }

        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        // Create the Direct3D 11 API device object and a corresponding context.
        RefCountPtr<ID3D11Device> device;
        RefCountPtr<ID3D11DeviceContext> context;

        HRESULT hr = E_FAIL;
        if (adapter)
        {
            hr = D3D11CreateDevice(
                adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                creationFlags,
                s_featureLevels,
                _countof(s_featureLevels),
                D3D11_SDK_VERSION,
                device.GetAddressOf(),
                &featureLevel,
                context.GetAddressOf()
            );
        }
#if defined(NDEBUG)
        else
        {
            LOGE("No Direct3D11 hardware device found");
        }
#else
        if (FAILED(hr))
        {
            // If the initialization fails, fall back to the WARP device.
            // For more information on WARP, see:
            // http://go.microsoft.com/fwlink/?LinkId=286690
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
                nullptr,
                creationFlags,
                s_featureLevels,
                _countof(s_featureLevels),
                D3D11_SDK_VERSION,
                device.GetAddressOf(),
                &featureLevel,
                context.GetAddressOf()
            );

            if (SUCCEEDED(hr))
            {
                OutputDebugStringA("Direct3D Adapter - WARP\n");
            }
        }
#endif

        ThrowIfFailed(hr);

#ifndef NDEBUG
        RefCountPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3dDebug))))
        {
            RefCountPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue))))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
                D3D11_MESSAGE_ID hide[] =
                {
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                };
                D3D11_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
                filter.DenyList.pIDList = hide;
                d3dInfoQueue->AddStorageFilterEntries(&filter);
            }
        }
#endif

        ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&d3dDevice)));
        ThrowIfFailed(context->QueryInterface(IID_PPV_ARGS(&immediateContext)));

        commandList = std::make_unique<D3D11_CommandList>(this, context.Get());

        D3D11_FEATURE_DATA_D3D11_OPTIONS2 options2;
        hr = device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &options2, sizeof(options2));
        if (SUCCEEDED(hr) && options2.ConservativeRasterizationTier >= D3D11_CONSERVATIVE_RASTERIZATION_TIER_1)
        {
            LOGD("CONSERVATIVE_RASTERIZATION");
        }

        // Create SwapChain
        {
            swapChainDesc = {};
            swapChainDesc.Width = presentationParameters.backBufferWidth;
            swapChainDesc.Height = presentationParameters.backBufferHeight;
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = presentationParameters.backBufferCount;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            fullScreenDesc = {};
            fullScreenDesc.RefreshRate.Numerator = 0;
            fullScreenDesc.RefreshRate.Denominator = 1;
            fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
            fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fullScreenDesc.Windowed = !presentationParameters.isFullScreen;

            // Create a SwapChain from a Win32 window.
            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
                device.Get(),
                static_cast<HWND>(window->GetPlatformHandle()),
                &swapChainDesc,
                &fullScreenDesc,
                nullptr,
                swapChain.ReleaseAndGetAddressOf()
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(dxgiFactory->MakeWindowAssociation(static_cast<HWND>(window->GetPlatformHandle()), DXGI_MWA_NO_ALT_ENTER));
#else
#endif
        }

        depthStencilFormat = presentationParameters.depthStencilFormat;

        AfterReset();

        return true;
    }

    void D3D11_Device::WaitIdle()
    {
        immediateContext->Flush();

        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query = D3D11_QUERY_EVENT;
        queryDesc.MiscFlags = 0;

        RefCountPtr<ID3D11Query> query;
        ThrowIfFailed(d3dDevice->CreateQuery(&queryDesc, &query));
        immediateContext->End(query.Get());

        BOOL result;
        while (immediateContext->GetData(query.Get(), &result, sizeof(result), 0) == S_FALSE);
        ALIMER_ASSERT(result == TRUE);
    }

    ICommandList* D3D11_Device::BeginFrame()
    {
        if (deviceLost)
        {
            return nullptr;
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc;
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
        if (SUCCEEDED(swapChain->GetDesc1(&newSwapChainDesc)) &&
            SUCCEEDED(swapChain->GetFullscreenDesc(&newFullScreenDesc)))
        {
            if (fullScreenDesc.Windowed != newFullScreenDesc.Windowed)
            {
            }
        }
#endif

        return commandList.get();
    }

    void D3D11_Device::EndFrame()
    {
        UINT presentFlags = 0;
        if (!vsyncEnabled && fullScreenDesc.Windowed && tearingSupported)
        {
            presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
        }

        HRESULT hr = swapChain->Present(vsyncEnabled ? 1 : 0, presentFlags);

        // If the device was removed either by a disconnection or a driver upgrade, we
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice->GetDeviceRemovedReason() : hr));
            OutputDebugStringA(buff);
#endif
            HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);

            frameCount++;

            if (!dxgiFactory->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                CreateFactory();
            }
        }
    }

    void D3D11_Device::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        AfterReset();
    }

    void D3D11_Device::AfterReset()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(swapChain->GetDesc1(&swapChainDesc));

        const TextureDesc backBufferTextureDesc = TextureDesc::Tex2D(Format::BGRA8UNorm,
            swapChainDesc.Width, swapChainDesc.Height, 1, 1, TextureUsage::RenderTarget
        );

        RefCountPtr<ID3D11Texture2D> d3d11BackBuffer;
        ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(d3d11BackBuffer.ReleaseAndGetAddressOf())));
        backBuffer = CreateExternalTexture(d3d11BackBuffer.Get(), backBufferTextureDesc);

        backBufferWidth = swapChainDesc.Width;
        backBufferHeight = swapChainDesc.Height;

        if (depthStencilFormat != Format::Undefined)
        {
            depthStencilTexture = CreateTexture(TextureDesc::Tex2D(depthStencilFormat, backBufferWidth, backBufferHeight, 1, 1, TextureUsage::RenderTarget));
        }
    }

    TextureHandle D3D11_Device::CreateTextureCore(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        auto result = new D3D11_Texture();
        result->device = this;
        result->desc = desc;

        if (desc.mipLevels == 0)
        {
            result->desc.mipLevels = (uint32_t)log2(std::max(desc.width, desc.height)) + 1;
        }

        result->dxgiFormat = ToDXGIFormat(desc.format);

        // External texture handle
        if (nativeHandle != nullptr)
        {
            switch (desc.dimension)
            {
                case TextureDimension::Texture2D:
                case TextureDimension::TextureCube:
                {
                    ID3D11Texture2D* d3d11Tex2D = (ID3D11Texture2D*)nativeHandle;
                    D3D11_TEXTURE2D_DESC desc2D;
                    d3d11Tex2D->GetDesc(&desc2D);

                    if (desc2D.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                    {
                        result->desc.usage |= TextureUsage::ShaderRead;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
                    {
                        result->desc.usage |= TextureUsage::ShaderWrite;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
                    {
                        result->desc.usage |= TextureUsage::RenderTarget;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_DEPTH_STENCIL)
                    {
                        result->desc.usage |= TextureUsage::RenderTarget;
                    }

                    result->handle = d3d11Tex2D;
                    result->handle->AddRef();
                }
                break;
            }

            return TextureHandle::Create(result);
        }

        HRESULT hr = E_FAIL;

        DXGI_FORMAT format = result->dxgiFormat;
        UINT bindFlags = 0;
        if (Any(desc.usage, TextureUsage::ShaderRead))
        {
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;

            if (IsDepthFormat(desc.format))
            {
                format = GetTypelessFormatFromDepthFormat(desc.format);
            }
        }

        if (Any(desc.usage, TextureUsage::ShaderWrite))
        {
            bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }

        if (Any(desc.usage, TextureUsage::RenderTarget))
        {
            if (IsDepthStencilFormat(desc.format))
            {
                bindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else
            {
                bindFlags |= D3D11_BIND_RENDER_TARGET;
            }
        }

        switch (desc.dimension)
        {
            case TextureDimension::Texture2D:
            case TextureDimension::Texture2DArray:
            case TextureDimension::TextureCube:
            case TextureDimension::TextureCubeArray:
            case TextureDimension::Texture2DMS:
            case TextureDimension::Texture2DMSArray:
            {
                D3D11_TEXTURE2D_DESC desc2D = {};
                desc2D.Width = desc.width;
                desc2D.Height = desc.height;
                desc2D.MipLevels = desc.mipLevels;
                desc2D.ArraySize = desc.depthOrArraySize;
                desc2D.Format = format;
                desc2D.SampleDesc.Count = desc.sampleCount;
                desc2D.SampleDesc.Quality = 0;
                desc2D.Usage = D3D11_USAGE_DEFAULT;
                desc2D.BindFlags = bindFlags;
                desc2D.CPUAccessFlags = 0;
                desc2D.MiscFlags = 0;

                if (desc.dimension == TextureDimension::TextureCube)
                {
                    desc2D.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
                }

                hr = d3dDevice->CreateTexture2D(&desc2D, nullptr, (ID3D11Texture2D**)&result->handle);
                break;
            }

            default:
                break;
        }

        if (FAILED(hr))
        {
            return nullptr;
        }

        return TextureHandle::Create(result);
    }

    BufferHandle D3D11_Device::CreateBufferCore(const BufferDesc& desc, void* nativeHandle, const void* initialData)
    {
        return nullptr;
    }

    ShaderHandle D3D11_Device::CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
        auto byteCode = CompileShader(stage, source, entryPoint);
        if (byteCode.empty())
            return nullptr;

        RefCountPtr<D3D11_Shader> shader = RefCountPtr<D3D11_Shader>::Create(new D3D11_Shader());
        shader->device = this;

        switch (stage)  // NOLINT(clang-diagnostic-switch-enum)
        {
            case ShaderStages::Vertex:
            {
                // Save the bytecode for potential input layout creation later
                shader->bytecode.resize(byteCode.size());
                memcpy(shader->bytecode.data(), byteCode.data(), byteCode.size());

                const HRESULT res = d3dDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &shader->VS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreateVertexShader");
                    return nullptr;
                }
            }
            break;
            case ShaderStages::Hull:
            {
                const HRESULT res = d3dDevice->CreateHullShader(byteCode.data(), byteCode.size(), nullptr, &shader->HS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreateHullShader");
                    return nullptr;
                }
            }
            break;
            case ShaderStages::Domain:
            {
                const HRESULT res = d3dDevice->CreateDomainShader(byteCode.data(), byteCode.size(), nullptr, &shader->DS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreateDomainShader");
                    return nullptr;
                }
            }
            break;
            case ShaderStages::Geometry:
            {
                const HRESULT res = d3dDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &shader->GS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreateGeometryShader");
                    return nullptr;
                }
            }
            break;
            case ShaderStages::Pixel:
            {
                const HRESULT res = d3dDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &shader->PS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreatePixelShader");
                    return nullptr;
                }
            }
            break;
            case ShaderStages::Compute:
            {
                const HRESULT res = d3dDevice->CreateComputeShader(byteCode.data(), byteCode.size(), nullptr, &shader->CS);
                if (FAILED(res))
                {
                    LOGE("Direct3D11: Failed to CreateComputeShader");
                    return nullptr;
                }
            }
            break;
            default:
                return nullptr;
        }

        return shader;
    }

    std::vector<uint8_t> D3D11_Device::CompileShader(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
#if defined(ALIMER_DISABLE_SHADER_COMPILER)
        return {};
#else
        if (!LoadShaderCompiler())
            return {};

        UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        std::string profile;
        switch (stage)
        {
            // clang-format off
            case ShaderStages::Vertex:           profile = "vs"; break;
            case ShaderStages::Hull:             profile = "hs"; break;
            case ShaderStages::Domain:           profile = "ds"; break;
            case ShaderStages::Geometry:         profile = "gs"; break;
            case ShaderStages::Pixel:            profile = "ps"; break;
            case ShaderStages::Compute:          profile = "cs"; break;
            default:
                ALIMER_UNREACHABLE();
                return {};
        }

        const uint32_t shaderModelMajor = 5;
        const uint32_t shaderModelMinor = 0;

        profile += "_";
        profile += std::to_string(shaderModelMajor);
        profile += "_";
        profile += std::to_string(shaderModelMinor);

        RefCountPtr<ID3DBlob> output;
        RefCountPtr<ID3DBlob> errors_or_warnings;

        HRESULT hr = D3DCompile(
            source.c_str(),                     /* pSrcData */
            source.length(),                    /* SrcDataSize */
            nullptr,                            /* pSourceName */
            NULL,                               /* pDefines */
            D3D_COMPILE_STANDARD_FILE_INCLUDE,  /* pInclude */
            entryPoint.c_str(),                 /* pEntryPoint */
            profile.c_str(),                    /* pTarget */
            compileFlags,                       /* Flags1 */
            0,                                  /* Flags2 */
            &output,                            /* ppCode */
            &errors_or_warnings);               /* ppErrorMsgs */

        if (errors_or_warnings)
        {
            LOGE((LPCSTR)errors_or_warnings->GetBufferPointer());
        }

        if (FAILED(hr))
        {
            return {};
        }

        std::vector<uint8_t> byteCode(output->GetBufferSize());
        //shader->bytecode.resize(byteCode.size());
        memcpy(byteCode.data(), output->GetBufferPointer(), output->GetBufferSize());
        return byteCode;
#endif
    }

#if !defined(ALIMER_DISABLE_SHADER_COMPILER)
    bool D3D11_Device::LoadShaderCompiler()
    {
#if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
        return true;
#else
        /* load DLL on demand */
        if (D3DCompiler == nullptr && !D3DCompiler_LoadFailed)
        {
            D3DCompiler = LoadLibraryW(L"D3DCompiler_47.dll");
            if (D3DCompiler == nullptr)
            {
                /* don't attempt to load missing DLL in the future */
                LOGD("Direct3D11: Failed to load D3DCompiler_47.dll!");
                D3DCompiler_LoadFailed = true;
                return false;
            }

            /* look up function pointers */
            D3DCompile = (pD3DCompile)(void*)GetProcAddress(D3DCompiler, "D3DCompile");
            ALIMER_ASSERT(D3DCompile != nullptr);
        }

        return D3DCompiler != nullptr;
#endif
    }
#endif

    PipelineHandle D3D11_Device::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        RefCountPtr<D3D11_Pipeline> pipeline = RefCountPtr<D3D11_Pipeline>::Create(new D3D11_Pipeline());
        pipeline->device = this;

        pipeline->shaderStages = ShaderStages::None;
        if (desc.vertex)
        {
            pipeline->vertex = checked_cast<D3D11_Shader*>(desc.vertex)->VS;
            pipeline->shaderStages |= ShaderStages::Vertex;
        }

        if (desc.hull)
        {
            pipeline->hull = checked_cast<D3D11_Shader*>(desc.hull)->HS;
            pipeline->shaderStages |= ShaderStages::Hull;
        }

        if (desc.domain)
        {
            pipeline->domain = checked_cast<D3D11_Shader*>(desc.domain)->DS;
            pipeline->shaderStages |= ShaderStages::Domain;
        }

        if (desc.geometry)
        {
            pipeline->geometry = checked_cast<D3D11_Shader*>(desc.geometry)->GS;
            pipeline->shaderStages |= ShaderStages::Geometry;
        }

        if (desc.pixel)
        {
            pipeline->pixel = checked_cast<D3D11_Shader*>(desc.pixel)->PS;
            pipeline->shaderStages |= ShaderStages::Pixel;
        }

        pipeline->blendState = GetBlendState(desc.blendState);
        pipeline->depthStencilState = GetDepthStencilState(desc.depthStencilState);
        pipeline->rasterizerState = GetRasterizerState(desc.rasterizerState);

        pipeline->primitiveTopology = ConvertPrimitiveTopology(desc.primitiveTopology, desc.patchControlPoints);

        return pipeline;
    }

    ID3D11BlendState1* D3D11_Device::GetBlendState(const BlendState& state)
    {
        std::hash<BlendState> hasher;
        size_t hash = hasher(state);

        RefCountPtr<ID3D11BlendState1> blendState = blendStates[hash];

        if (blendState)
            return blendState;

        D3D11_BLEND_DESC1 desc = {};
        desc.AlphaToCoverageEnable = state.alphaToCoverageEnable ? TRUE : FALSE;
        desc.IndependentBlendEnable = state.independentBlendEnable ? TRUE : FALSE;

        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderTargetBlendState& renderTarget = state.renderTargets[i];

            desc.RenderTarget[i].BlendEnable = renderTarget.blendEnable ? TRUE : FALSE;
            desc.RenderTarget[i].SrcBlend = D3D11Blend(renderTarget.srcBlend);
            desc.RenderTarget[i].DestBlend = D3D11Blend(renderTarget.destBlend);
            desc.RenderTarget[i].BlendOp = D3D11BlendOperation(renderTarget.blendOp);
            desc.RenderTarget[i].SrcBlendAlpha = D3D11AlphaBlend(renderTarget.srcBlendAlpha);
            desc.RenderTarget[i].DestBlendAlpha = D3D11AlphaBlend(renderTarget.destBlendAlpha);
            desc.RenderTarget[i].BlendOpAlpha = D3D11BlendOperation(renderTarget.blendOpAlpha);
            desc.RenderTarget[i].RenderTargetWriteMask = (UINT8)D3D11RenderTargetWriteMask(renderTarget.writeMask);
            desc.RenderTarget[i].LogicOpEnable = false;
            desc.RenderTarget[i].LogicOp = D3D11_LOGIC_OP_NOOP;
        }

        const HRESULT hr = d3dDevice->CreateBlendState1(&desc, &blendState);
        if (FAILED(hr))
        {
            LOGE("Direct3D11: Failed to create blend state");
            return nullptr;
        }

        blendStates[hash] = blendState;
        return blendState;
    }

    ID3D11DepthStencilState* D3D11_Device::GetDepthStencilState(const DepthStencilState& state)
    {
        std::hash<DepthStencilState> hasher;
        size_t hash = hasher(state);

        RefCountPtr<ID3D11DepthStencilState> depthStencilState = depthStencilStates[hash];

        if (depthStencilState)
            return depthStencilState;

        D3D11_DEPTH_STENCIL_DESC desc;
        desc.DepthEnable = (state.depthCompare != CompareFunction::Always || state.depthWriteEnable) ? TRUE : FALSE;
        desc.DepthWriteMask = state.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = ToD3D11(state.depthCompare);
        desc.StencilEnable = state.stencilEnable ? TRUE : FALSE;
        desc.StencilReadMask = (UINT8)state.stencilReadMask;
        desc.StencilWriteMask = (UINT8)state.stencilWriteMask;
        desc.FrontFace.StencilFailOp = ToD3D11(state.frontFace.failOp);
        desc.FrontFace.StencilDepthFailOp = ToD3D11(state.frontFace.depthFailOp);
        desc.FrontFace.StencilPassOp = ToD3D11(state.frontFace.passOp);
        desc.FrontFace.StencilFunc = ToD3D11(state.frontFace.compare);
        desc.BackFace.StencilFailOp = ToD3D11(state.backFace.failOp);
        desc.BackFace.StencilDepthFailOp = ToD3D11(state.backFace.depthFailOp);
        desc.BackFace.StencilPassOp = ToD3D11(state.backFace.passOp);
        desc.BackFace.StencilFunc = ToD3D11(state.backFace.compare);

        const HRESULT hr = d3dDevice->CreateDepthStencilState(&desc, &depthStencilState);
        if (FAILED(hr))
        {
            LOGE("Direct3D11: Failed to create DepthStencil state");
            return nullptr;
        }

        depthStencilStates[hash] = depthStencilState;
        return depthStencilState.Get();
    }

    ID3D11RasterizerState1* D3D11_Device::GetRasterizerState(const RasterizerState& state)
    {
        std::hash<RasterizerState> hasher;
        size_t hash = hasher(state);

        RefCountPtr<ID3D11RasterizerState1> rasterizerState = rasterizerStates[hash];

        if (rasterizerState)
            return rasterizerState;

        D3D11_RASTERIZER_DESC1 desc;
        desc.FillMode = D3D11_FILL_SOLID;
        switch (state.fillMode)
        {
            case FillMode::Wireframe:
                desc.FillMode = D3D11_FILL_WIREFRAME;
                break;
            default:
                break;
        }

        desc.CullMode = D3D11_CULL_BACK;
        switch (state.cullMode)
        {
            case CullMode::Front:
                desc.CullMode = D3D11_CULL_FRONT;
                break;
            case CullMode::None:
                desc.CullMode = D3D11_CULL_NONE;
                break;
            default:
                break;
        }

        desc.FrontCounterClockwise = (state.frontFace == FaceWinding::CounterClockwise) ? TRUE : FALSE;
        desc.DepthBias = FloorToInt(state.depthBias * (float)(1 << 24));
        desc.DepthBiasClamp = state.depthBiasClamp;
        desc.SlopeScaledDepthBias = state.depthBiasSlopeScale;
        desc.DepthClipEnable = TRUE; // state.depthClipEnable ? TRUE : FALSE;
        desc.ScissorEnable = TRUE;
        desc.MultisampleEnable = TRUE;
        desc.AntialiasedLineEnable = FALSE;
        desc.ForcedSampleCount = 0;

        const HRESULT hr = d3dDevice->CreateRasterizerState1(&desc, &rasterizerState);
        if (FAILED(hr))
        {
            LOGE("Direct3D11: Failed to create Rasterizer state");
            return nullptr;
        }

        rasterizerStates[hash] = rasterizerState;
        return rasterizerState.Get();
    }

    void D3D11_Device::CreateFactory()
    {
#if defined(_DEBUG) && (_WIN32_WINNT >= 0x0603 /*_WIN32_WINNT_WINBLUE*/)
        bool debugDXGI = false;
        {
            RefCountPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                debugDXGI = true;

                ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };
                DXGI_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
                filter.DenyList.pIDList = hide;
                dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
            }
        }

        if (!debugDXGI)
#endif
        {
            ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));
        }
    }

    void D3D11_Device::GetAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        RefCountPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        RefCountPtr<IDXGIFactory6> dxgiFactory6;
        if (SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory6))))
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(dxgiFactory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                    IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif

                break;
            }
        }
#endif
        if (!adapter)
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(dxgiFactory->EnumAdapters1(
                    adapterIndex,
                    adapter.ReleaseAndGetAddressOf()));
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif

                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }

    void D3D11_Device::HandleDeviceLost()
    {
        // TODO
    }

    DeviceHandle CreateD3D11Device(alimer::Window* window, const PresentationParameters& presentationParameters)
    {
        auto device = new D3D11_Device();
        if (device->Initialize(window, presentationParameters))
        {
            return DeviceHandle::Create(device);
        }

        delete device;
        return nullptr;
    }
}

#endif /* defined(ALIMER_RHI_D3D11) */
