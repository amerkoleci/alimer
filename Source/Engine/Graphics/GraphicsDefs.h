// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Graphics/PixelFormat.h"
#include "Math/Color.h"

namespace Alimer
{
    /* Constants */
    static constexpr uint32_t KnownVendorId_Nvidia = 0x010DE;
    static constexpr uint32_t KnownVendorId_AMD = 0x01002;
    static constexpr uint32_t KnownVendorId_Intel = 0x08086;
    static constexpr uint32_t KnownVendorId_ARM = 0x013B5;
    static constexpr uint32_t KnownVendorId_Qualcomm = 0x05143;
    static constexpr uint32_t KnownVendorId_ImgTec = 0x01010;
    static constexpr uint32_t KnownVendorId_Microsoft = 0x01414;
    static constexpr uint32_t KnownVendorId_Apple = 0x0106B;
    static constexpr uint32_t KnownVendorId_Mesa = 0x10005;
    static constexpr uint32_t KnownVendorId_BROADCOM = 0x014e4;

    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 8;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxCommandLists = 32u;

    static constexpr uint32_t kMaxConstantBufferBindings = 15;
    static constexpr uint32_t kMaxSRVBindings = 64u;
    static constexpr uint32_t kMaxSamplerBindings = 16;
    //static constexpr uint32_t kMaxDescriptorBindings = 32;

    static constexpr uint32_t kInvalidBindlessIndex = static_cast<u32>(-1);
    static constexpr uint32_t kAllMipLevels = static_cast<u32>(-1);
    static constexpr uint32_t kAllArraySlices = static_cast<u32>(-1);

    /* Enums */
    enum class GraphicsAPI : uint8_t
    {
        Default = 0,
        D3D12,
        Vulkan,
        Metal,
        Null
    };

    enum class ShaderFormat : uint8_t
    {
        DXIL = 0,
        SPIRV,
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

    enum class CpuAccessMode : uint8_t
    {
        None,
        Write,
        Read
    };

    /// Number of MSAA samples to use. 1xMSAA and 4xMSAA are most broadly supported
    enum class SampleCount : uint32_t
    {
        Count1,
        Count2,
        Count4,
        Count8,
        Count16,
        Count32,
    };

    enum class QueueType : uint8_t
    {
        Graphics,
        Compute,
        Count
    };

    enum class CompareFunction : uint32_t
    {
        Undefined = 0,
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
        Uint8x2,
        Uint8x4,
        Sint8x2,
        Sint8x4,
        Unorm8x2,
        Unorm8x4,
        Snorm8x2,
        Snorm8x4,
        Uint16x2,
        Uint16x4,
        Sint16x2,
        Sint16x4,
        Unorm16x2,
        Unorm16x4,
        Snorm16x2,
        Snorm16x4,
        Float16x2,
        Float16x4,
        Float32,
        Float32x2,
        Float32x3,
        Float32x4,
        Uint32,
        Uint32x2,
        Uint32x3,
        Uint32x4,
        Sint32,
        Sint32x2,
        Sint32x3,
        Sint32x4,
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

    enum class ShaderStages : uint32_t
    {
        None = 0x0000,

        Compute = 0x0020,

        Vertex = 0x0001,
        Hull = 0x0002,
        Domain = 0x0004,
        Geometry = 0x0008,
        Pixel = 0x0010,
        Amplification = 0x0040,
        Mesh = 0x0080,
        AllGraphics = 0x00FE,

        RayGeneration = 0x0100,
        AnyHit = 0x0200,
        ClosestHit = 0x0400,
        Miss = 0x0800,
        Intersection = 0x1000,
        Callable = 0x2000,
        AllRayTracing = 0x3F00,

        All = 0x3FFF,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ShaderStages);

    enum class PresentMode : uint32_t
    {
        Immediate = 0,
        Mailbox,
        Fifo
    };

    /* Forward declarations */
    class Buffer;
    class Texture;
    class TextureView;
    class Sampler;
    class Shader;
    class Pipeline;
    class SwapChain;

    using BufferRef = RefPtr<Buffer>;
    using TextureRef = RefPtr<Texture>;
    using SamplerRef = RefPtr<Sampler>;
    using ShaderRef = RefPtr<Shader>;
    using PipelineRef = RefPtr<Pipeline>;
    using SwapChainRef = RefPtr<SwapChain>;

    /* Structs */
    struct DispatchIndirectCommand {
        uint32_t    x;
        uint32_t    y;
        uint32_t    z;
    };

    struct DrawIndexedIndirectCommand {
        uint32_t    indexCount;
        uint32_t    instanceCount;
        uint32_t    firstIndex;
        int32_t     vertexOffset;
        uint32_t    firstInstance;
    };

    struct DrawIndirectCommand {
        uint32_t    vertexCount;
        uint32_t    instanceCount;
        uint32_t    firstVertex;
        uint32_t    firstInstance;
    };

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

    struct GraphicsDeviceFeatures
    {
        /// Whether Ray Tracing support is available.
        bool rayTracing = false;
        /// Whether Mesh Shader support is available.
        bool meshShader = false;
        /// Whether VariableRate shading support is available.
        bool variableRateShading = false;
        /// Whether VariableRate shading extended support is available.
        bool variableRateShadingTier2 = false;
    };

    struct GraphicsDeviceLimits
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

    struct GraphicsDeviceCaps
    {
        GraphicsAPI			    backendType;
        GPUVendorId             vendor = GPUVendorId::Unknown;
        uint32_t				vendorId = 0;
        uint32_t				adapterId = 0;
        GPUAdapterType			adapterType = GPUAdapterType::Unknown;
        std::string				adapterName;
        GraphicsDeviceFeatures	features;
        GraphicsDeviceLimits	limits;
    };

    /* Helper methods */
    ALIMER_API const char* GetVendorName(uint32_t vendorId);
    ALIMER_API GPUVendorId VendorIdToAdapterVendor(uint32_t vendorId);
    ALIMER_API const char* ToString(GPUAdapterType type);

    ALIMER_API const char* ToString(CompareFunction func);

    enum class VertexFormatBaseType
    {
        Float,
        Uint,
        Sint,
    };

    struct VertexFormatInfo
    {
        VertexFormat format;
        uint32_t byteSize;
        uint32_t componentCount;
        uint32_t componentByteSize;
        VertexFormatBaseType baseType;
    };

    ALIMER_API const VertexFormatInfo& GetVertexFormatInfo(VertexFormat format);
}

