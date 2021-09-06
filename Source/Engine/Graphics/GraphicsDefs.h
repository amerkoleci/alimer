// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/PixelFormat.h"
#include "Math/Color.h"

namespace Alimer
{
    /* Constants */
    static constexpr u32 kMaxFramesInFlight = 2;
    static constexpr u32 kMaxColorAttachments = 8;
    static constexpr u32 kMaxViewportsAndScissors = 8;
    static constexpr u32 kMaxVertexBufferBindings = 8;
    static constexpr u32 kMaxVertexAttributes = 16;
    static constexpr u32 kMaxVertexAttributeOffset = 2047u;
    static constexpr u32 kMaxVertexBufferStride = 2048u;
    static constexpr u32 kMaxCommandLists = 32u;
    //static constexpr uint32_t kMaxUniformBufferBindings = 14;
    //static constexpr uint32_t kMaxDescriptorBindings = 32;
    //static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;

    static constexpr u32 kInvalidBindlessIndex = static_cast<u32>(-1);
    static constexpr u32 kAllMipLevels = static_cast<u32>(-1);
    static constexpr u32 kAllArraySlices = static_cast<u32>(-1);

    static constexpr u32 KnownVendorId_AMD = 0x1002;
    static constexpr u32 KnownVendorId_Intel = 0x8086;
    static constexpr u32 KnownVendorId_Nvidia = 0x10DE;
    static constexpr u32 KnownVendorId_Microsoft = 0x1414;
    static constexpr u32 KnownVendorId_ARM = 0x13B5;
    static constexpr u32 KnownVendorId_ImgTec = 0x1010;
    static constexpr u32 KnownVendorId_Qualcomm = 0x5143;

    /* Enums */
    enum class GraphicsAPI : uint8_t
    {
        D3D11,
        D3D12,
        Vulkan
    };

    enum class ValidationMode : uint32_t
    {
        /// No validation is enabled.
        Disabled,
        /// Print warnings and errors
        Enabled,
        /// Print all warnings, errors and info messages
        Verbose,
        /// Enable GPU-based validation
        GPU
    };

    enum class PrimitiveTopology : u32
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        PatchList,
        Count
    };

    enum class VertexElementUsage : u32
    {
        Position,
        Normal,
        Tangent,
        Color,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        TexCoord4,
        TexCoord5,
        TexCoord6,
        TexCoord7,
        BlendWeight,
        BlendIndices,

        Count
    };

    enum class VertexElementFormat : uint32_t
    {
        UChar2,
        UChar4,
        Char2,
        Char4,
        UChar2Norm,
        UChar4Norm,
        Char2Norm,
        Char4Norm,
        UShort2,
        UShort4,
        Short2,
        Short4,
        UShort2Norm,
        UShort4Norm,
        Short2Norm,
        Short4Norm,
        Half2,
        Half4,
        Float,
        Float2,
        Float3,
        Float4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
        Int,
        Int2,
        Int3,
        Int4,
        RGB10A2Unorm
    };

    enum class VertexStepRate : uint32_t
    {
        Vertex = 0,
        Instance
    };

    enum class IndexType : uint32_t
    {
        UInt16 = 0,
        UInt32
    };

    /* Structs */
    struct VertexElement
    {
        VertexElementFormat format = VertexElementFormat::Float3;
        VertexElementUsage usage = VertexElementUsage::Position;
        uint32_t offset = 0;
        uint32_t bufferIndex = 0;
        VertexStepRate stepRate = VertexStepRate::Vertex;
    };

    struct PresentationParameters
    {
        ValidationMode validationMode = ValidationMode::Disabled;

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        uint32_t backBufferCount = 3;
        PixelFormat depthStencilFormat = PixelFormat::Depth32Float;
        bool vsyncEnabled = false;
        bool isFullScreen = false;
    };

    struct DeviceFeatures
    {
        /// Whether Ray Tracing support is available.
        bool rayTracing = false;
        /// Whether Mesh Shader support is available.
        bool meshShader = false;
    };

    struct DeviceLimits
    {
        /// The maximum pixel size of a 1d image.
        uint32_t maxTextureDimension1D;

        /// The maximum pixel size along one axis of a 2d image.
        uint32_t maxTextureDimension2D;

        /// The maximum pixel size along one axis of a 3d image.
        uint32_t maxTextureDimension3D;

        /// The maximum pixel size along one axis of a cube image.
        uint32_t maxTextureDimensionCube;

        /// The maximum size of an image array.
        uint32_t maxTextureArraySize;

        /// The alignment required when creating buffer views
        uint32_t minConstantBufferOffsetAlignment;

        /// The maximum number of draws when doing indirect drawing.
        uint32_t maxDrawIndirectCount = 1;
    };

    /* Helper methods */
    inline const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case KnownVendorId_AMD:
                return "AMD";
            case KnownVendorId_ImgTec:
                return "IMAGINATION";
            case KnownVendorId_Nvidia:
                return "Nvidia";
            case KnownVendorId_ARM:
                return "ARM";
            case KnownVendorId_Qualcomm:
                return "Qualcom";
            case KnownVendorId_Intel:
                return "Intel";
            default:
                return "Unknown";
        }
    }

    ALIMER_API uint32_t GetVertexFormatNumComponents(VertexElementFormat format);
    ALIMER_API uint32_t GetVertexFormatComponentSize(VertexElementFormat format);
    ALIMER_API uint32_t GetVertexFormatSize(VertexElementFormat format);
}

