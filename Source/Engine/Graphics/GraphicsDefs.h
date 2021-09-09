// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
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
    static constexpr u32 kMaxUniformBufferBindings = 14;
    //static constexpr uint32_t kMaxDescriptorBindings = 32;
    //static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;

    static constexpr u32 kInvalidBindlessIndex = static_cast<u32>(-1);
    static constexpr u32 kAllMipLevels = static_cast<u32>(-1);
    static constexpr u32 kAllArraySlices = static_cast<u32>(-1);

    /* Enums */
    enum class GraphicsAPI : uint8_t
    {
        D3D11,
        D3D12,
        Vulkan
    };

    enum class ShaderFormat : uint8_t
    {
        DXIL,
        SPIRV
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

    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,

        Count
    };

    enum class HeapType : uint32_t
    {
        Default,
        Upload,
        Readback,
    };

    enum class ResourceStates : uint32_t
    {
        Unknown = 0,
        Common = 0x00000001,
        ConstantBuffer = 0x00000002,
        VertexBuffer = 0x00000004,
        IndexBuffer = 0x00000008,
        IndirectArgument = 0x00000010,
        ShaderResource = 0x00000020,
        UnorderedAccess = 0x00000040,
        RenderTarget = 0x00000080,
        DepthWrite = 0x00000100,
        DepthRead = 0x00000200,
        StreamOut = 0x00000400,
        CopyDest = 0x00000800,
        CopySource = 0x00001000,
        ResolveDest = 0x00002000,
        ResolveSource = 0x00004000,
        Present = 0x00008000,
        AccelerationStructureRead = 0x00010000,
        AccelerationStructureWrite = 0x00020000,
        AccelerationStructureBuildInput = 0x00040000,
        AccelerationStructureBuildBlas = 0x00080000,
        ShadingRateSurface = 0x00100000,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ResourceStates);

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
    class Buffer;
    class Texture;
    class TextureView;
    class Sampler;
    class Shader;
    class Pipeline;
    class CommandBuffer;

    using BufferRef = RefCountPtr<Buffer>;
    using TextureRef = RefCountPtr<Texture>;
    using SamplerRef = RefCountPtr<Sampler>;
    using ShaderRef = RefCountPtr<Shader>;
    using PipelineRef = RefCountPtr<Pipeline>;

    /* Structs */
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

        /// The alignment required for constant buffers.
        uint64_t minConstantBufferOffsetAlignment;

        /// The alignment required for storage buffers.
        uint64_t minStorageBufferOffsetAlignment;

        /// The maximum number of draws when doing indirect drawing.
        uint32_t maxDrawIndirectCount = 1;
    };

    /* Helper methods */
    
    // Returns the number of mip levels given a texture size
    ALIMER_API uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth = 1);

    ALIMER_API const char* ToString(CompareFunction func);
}

