// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Graphics/PixelFormat.h"
#include "Math/Color.h"

namespace Alimer
{
    /* Constants */
    static constexpr u32 KnownVendorId_Nvidia = 0x010DE;
    static constexpr u32 KnownVendorId_AMD = 0x01002;
    static constexpr u32 KnownVendorId_Intel = 0x08086;
    static constexpr u32 KnownVendorId_ARM = 0x013B5;
    static constexpr u32 KnownVendorId_Qualcomm = 0x05143;
    static constexpr u32 KnownVendorId_ImgTec = 0x01010;
    static constexpr u32 KnownVendorId_Microsoft = 0x01414;
    static constexpr u32 KnownVendorId_Apple = 0x0106B;
    static constexpr u32 KnownVendorId_Mesa = 0x10005;
    static constexpr u32 KnownVendorId_BROADCOM = 0x014e4;

    static constexpr u32 kMaxFramesInFlight = 2;
    static constexpr u32 kMaxColorAttachments = 8;
    static constexpr u32 kMaxViewportsAndScissors = 8;
    static constexpr u32 kMaxVertexBufferBindings = 8;
    static constexpr u32 kMaxVertexAttributes = 16;
    static constexpr u32 kMaxVertexAttributeOffset = 2047u;
    static constexpr u32 kMaxVertexBufferStride = 2048u;
    static constexpr u32 kMaxCommandLists = 32u;
    static constexpr u32 kMaxUniformBufferBindings = 14;
    //static constexpr uint32_t kMaxDescriptorBindings = 32;
    //static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;

    static constexpr u32 kInvalidBindlessIndex = static_cast<u32>(-1);
    static constexpr u32 kAllMipLevels = static_cast<u32>(-1);
    static constexpr u32 kAllArraySlices = static_cast<u32>(-1);

    /* Enums */
    enum class GraphicsAPI : uint8_t
    {
        Default = 0,
        D3D11,
        D3D12,
        OpenGL,
        Vulkan,
        Metal,
        Null
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

    enum class GPUAdapterType : uint8_t
    {
        Unknown,
        Software,
        Integrated,
        Discrete,
    };

    enum class GPUVendorId : uint8_t
    {
        /// Adapter vendor is unknown
        Unknown = 0,
        /// Adapter vendor is NVidia
        NVIDIA,
        /// Adapter vendor is AMD
        AMD,
        /// Adapter vendor is Intel
        INTEL,
        /// Adapter vendor is ARM
        ARM,
        /// Adapter vendor is Qualcomm
        QUALCOMM,
        /// Adapter vendor is Imagination Technologies
        IMGTECH,
        /// Adapter vendor is Microsoft (software rasterizer)
        MSFT,
        /// Adapter vendor is Apple
        APPLE,
        /// Adapter vendor is Mesa (software rasterizer)
        MESA,
        /// Adapter vendor is Broadcom (Raspberry Pi)
        BROADCOM,

    };

    enum class CompareFunction : uint32_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    enum class VertexFormat : uint32_t
    {
        Undefined = 0,
        UByte,
        UByte2,
        UByte4,
        Byte,
        Byte2,
        Byte4,
        UByteNorm,
        UByte2Norm,
        UByte4Norm,
        ByteNorm,
        Byte2Norm,
        Byte4Norm,
        UShort,
        UShort2,
        UShort4,
        Short,
        Short2,
        Short4,
        UShortNorm,
        UShort2Norm,
        UShort4Norm,
        ShortNorm,
        Short2Norm,
        Short4Norm,
        Half,
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

    enum class IndexType : uint32_t
    {
        UInt16 = 0,
        UInt32
    };

    enum class LoadAction : uint32_t
    {
        Clear,
        Load,
        Discard,
    };

    enum class StoreAction : uint32_t
    {
        Store,
        Discard,
    };

    /* Forward declarations */
    class Texture;
    class TextureView;
    class Sampler;
    class Shader;
    class Pipeline;
    class CommandContext;

    using TextureRef = RefCountPtr<Texture>;
    using SamplerRef = RefCountPtr<Sampler>;
    using ShaderRef = RefCountPtr<Shader>;
    using PipelineRef = RefCountPtr<Pipeline>;

    /* Structs */
    struct GraphicsAdapterInfo
    {
        std::string name;
        GPUAdapterType type = GPUAdapterType::Unknown;
        /// Adapter type
        /// Adapter vendor
        GPUVendorId vendor = GPUVendorId::Unknown;
        /// The PCI ID of the hardware vendor (if available).
        u32 vendorId = 0;
        /// The PCI ID of the hardware device (if available).
        u32 deviceId = 0;
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

        /// The alignment required for constant buffers.
        uint64_t minConstantBufferOffsetAlignment;

        /// The alignment required for storage buffers.
        uint64_t minStorageBufferOffsetAlignment;

        /// The maximum number of draws when doing indirect drawing.
        uint32_t maxDrawIndirectCount = 1;
    };

    /* Helper methods */
    ALIMER_API const char* GetVendorName(uint32_t vendorId);
    ALIMER_API GPUVendorId VendorIdToAdapterVendor(uint32_t vendorId);

    ALIMER_API const char* ToString(CompareFunction func);
}

