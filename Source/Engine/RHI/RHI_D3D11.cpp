// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_D3D11)
#include "Window.h"
#include "Core/Log.h"
#include "RHI_D3D11.h"
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
    DXGI_FORMAT ToDXGIFormat(Format format)
    {
        switch (format)
        {
            // 8-bit formats
            case Format::R8UNorm:  return DXGI_FORMAT_R8_UNORM;
            case Format::R8SNorm:  return DXGI_FORMAT_R8_SNORM;
            case Format::R8UInt:   return DXGI_FORMAT_R8_UINT;
            case Format::R8SInt:   return DXGI_FORMAT_R8_SINT;
                // 16-bit formats
            case Format::R16UNorm:     return DXGI_FORMAT_R16_UNORM;
            case Format::R16SNorm:     return DXGI_FORMAT_R16_SNORM;
            case Format::R16UInt:      return DXGI_FORMAT_R16_UINT;
            case Format::R16SInt:      return DXGI_FORMAT_R16_SINT;
            case Format::R16Float:     return DXGI_FORMAT_R16_FLOAT;
            case Format::RG8UNorm:     return DXGI_FORMAT_R8G8_UNORM;
            case Format::RG8SNorm:     return DXGI_FORMAT_R8G8_SNORM;
            case Format::RG8UInt:      return DXGI_FORMAT_R8G8_UINT;
            case Format::RG8SInt:      return DXGI_FORMAT_R8G8_SINT;
                // Packed 16-Bit Pixel Formats
            case Format::BGRA4UNorm:       return DXGI_FORMAT_B4G4R4A4_UNORM;
            case Format::B5G6R5UNorm:      return DXGI_FORMAT_B5G6R5_UNORM;
            case Format::B5G5R5A1UNorm:    return DXGI_FORMAT_B5G5R5A1_UNORM;
                // 32-bit formats
            case Format::R32UInt:          return DXGI_FORMAT_R32_UINT;
            case Format::R32SInt:          return DXGI_FORMAT_R32_SINT;
            case Format::R32Float:         return DXGI_FORMAT_R32_FLOAT;
            case Format::RG16UNorm:        return DXGI_FORMAT_R16G16_UNORM;
            case Format::RG16SNorm:        return DXGI_FORMAT_R16G16_SNORM;
            case Format::RG16UInt:         return DXGI_FORMAT_R16G16_UINT;
            case Format::RG16SInt:         return DXGI_FORMAT_R16G16_SINT;
            case Format::RG16Float:        return DXGI_FORMAT_R16G16_FLOAT;
            case Format::RGBA8UNorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Format::RGBA8UNormSrgb:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case Format::RGBA8SNorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
            case Format::RGBA8UInt:        return DXGI_FORMAT_R8G8B8A8_UINT;
            case Format::RGBA8SInt:        return DXGI_FORMAT_R8G8B8A8_SINT;
            case Format::BGRA8UNorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::BGRA8UNormSrgb:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                // Packed 32-Bit formats
            case Format::RGB10A2UNorm:     return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Format::RG11B10Float:     return DXGI_FORMAT_R11G11B10_FLOAT;
            case Format::RGB9E5Float:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                // 64-Bit formats
            case Format::RG32UInt:         return DXGI_FORMAT_R32G32_UINT;
            case Format::RG32SInt:         return DXGI_FORMAT_R32G32_SINT;
            case Format::RG32Float:        return DXGI_FORMAT_R32G32_FLOAT;
            case Format::RGBA16UNorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
            case Format::RGBA16SNorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
            case Format::RGBA16UInt:       return DXGI_FORMAT_R16G16B16A16_UINT;
            case Format::RGBA16SInt:       return DXGI_FORMAT_R16G16B16A16_SINT;
            case Format::RGBA16Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
                // 128-Bit formats
            case Format::RGBA32UInt:       return DXGI_FORMAT_R32G32B32A32_UINT;
            case Format::RGBA32SInt:       return DXGI_FORMAT_R32G32B32A32_SINT;
            case Format::RGBA32Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
                // Depth-stencil formats
            case Format::Depth16UNorm:			return DXGI_FORMAT_D16_UNORM;
            case Format::Depth32Float:			return DXGI_FORMAT_D32_FLOAT;
            case Format::Depth24UNormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Format::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                // Compressed BC formats
            case Format::BC1UNorm:         return DXGI_FORMAT_BC1_UNORM;
            case Format::BC1UNormSrgb:     return DXGI_FORMAT_BC1_UNORM_SRGB;
            case Format::BC2UNorm:         return DXGI_FORMAT_BC2_UNORM;
            case Format::BC2UNormSrgb:     return DXGI_FORMAT_BC2_UNORM_SRGB;
            case Format::BC3UNorm:         return DXGI_FORMAT_BC3_UNORM;
            case Format::BC3UNormSrgb:     return DXGI_FORMAT_BC3_UNORM_SRGB;
            case Format::BC4SNorm:         return DXGI_FORMAT_BC4_SNORM;
            case Format::BC4UNorm:         return DXGI_FORMAT_BC4_UNORM;
            case Format::BC5SNorm:         return DXGI_FORMAT_BC5_SNORM;
            case Format::BC5UNorm:         return DXGI_FORMAT_BC5_UNORM;
            case Format::BC6HUFloat:       return DXGI_FORMAT_BC6H_UF16;
            case Format::BC6HSFloat:       return DXGI_FORMAT_BC6H_SF16;
            case Format::BC7UNorm:         return DXGI_FORMAT_BC7_UNORM;
            case Format::BC7UNormSrgb:     return DXGI_FORMAT_BC7_UNORM_SRGB;

            default:
                ALIMER_UNREACHABLE();
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    namespace
    {
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
    }

    /* D3D11_Texture */
    D3D11_Texture::D3D11_Texture(D3D11_Device* device_, void* nativeHandle, const TextureDesc& desc_, const TextureData* initialData)
        : device(device_)
        , desc(desc_)
    {
        if (desc.mipLevels == 0)
        {
            desc.mipLevels = (uint32_t)log2(std::max(desc_.width, desc_.height)) + 1;
        }

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
                        desc.usage |= TextureUsage::ShaderRead;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
                    {
                        desc.usage |= TextureUsage::ShaderWrite;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
                    {
                        desc.usage |= TextureUsage::RenderTarget;
                    }

                    if (desc2D.BindFlags & D3D11_BIND_DEPTH_STENCIL)
                    {
                        desc.usage |= TextureUsage::RenderTarget;
                    }

                    handle = d3d11Tex2D;
                }
                break;
            }
        }
        else
        {
            HRESULT hr = E_FAIL;

            dxgiFormat = ToDXGIFormat(desc.format);
            DXGI_FORMAT format = dxgiFormat;
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

                    hr = device->GetD3DDevice()->CreateTexture2D(&desc2D, nullptr, (ID3D11Texture2D**)handle.ReleaseAndGetAddressOf());
                    break;
                }

                default:
                    break;
            }

            if (FAILED(hr))
            {
                return;
            }
        }
    }

    D3D11_Texture::~D3D11_Texture()
    {
        shaderResourceViews.clear();
        renderTargetViews.clear();
        depthStencilViews.clear();
        unorderedAccessViews.clear();
        handle.Reset();
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

            HRESULT hr = device->GetD3DDevice()->CreateRenderTargetView(handle.Get(), &viewDesc, &view);
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

            HRESULT hr = device->GetD3DDevice()->CreateDepthStencilView(handle.Get(), &viewDesc, &view);
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

                case LoadAction::DontCare:
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

                case LoadAction::DontCare:
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

                case LoadAction::DontCare:
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
                case StoreAction::DontCare:
                {
                    auto RTV = d3d11Texture->GetRTV(attachment.mipLevel, attachment.slice, 1);
                    context->DiscardView(RTV);
                    break;
                }

                case StoreAction::Resolve:
                case StoreAction::StoreAndResolve:
                {
                    auto resolveTexture = checked_cast<D3D11_Texture*>(attachment.resolveTexture);
                    uint32_t dstSubresource = D3D11CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, resolveTexture->desc.mipLevels);
                    uint32_t srcSubresource = D3D11CalcSubresource(attachment.mipLevel, attachment.slice, d3d11Texture->desc.mipLevels);
                    context->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                    break;
                }

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
                case StoreAction::DontCare:
                {
                    auto DSV = d3d11Texture->GetDSV(attachment.mipLevel, attachment.slice, 1);
                    context->DiscardView(DSV);
                    break;
                }

                case StoreAction::Resolve:
                case StoreAction::StoreAndResolve:
                {
                    auto resolveTexture = checked_cast<D3D11_Texture*>(attachment.resolveTexture);
                    uint32_t dstSubresource = D3D11CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, resolveTexture->desc.mipLevels);
                    uint32_t srcSubresource = D3D11CalcSubresource(attachment.mipLevel, attachment.slice, d3d11Texture->desc.mipLevels);
                    context->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                    break;
                }

                default:
                    break;
            }
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
        backBuffer.Reset();
        depthStencilTexture.Reset();
        swapChain.Reset();

        commandList.reset();
        immediateContext.Reset();
        d3dDevice.Reset();
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

    TextureHandle D3D11_Device::CreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        auto result = new D3D11_Texture(this, nullptr, desc, initialData);

        if (result->handle)
            return TextureHandle::Create(result);

        delete result;
        return nullptr;
    }

    TextureHandle D3D11_Device::CreateExternalTexture(void* nativeHandle, const TextureDesc& desc)
    {
        auto result = new D3D11_Texture(this, nativeHandle, desc, nullptr);

        if (result->handle)
            return TextureHandle::Create(result);

        delete result;
        return nullptr;
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
