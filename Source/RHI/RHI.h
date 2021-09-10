// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <atomic>
#include <type_traits>

#define RHI_ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

#if defined(RHI_SHARED_LIBRARY_BUILD)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllexport)
#   elif defined(__GNUC__)
#       define RHI_API __attribute__((visibility("default")))
#   else
#       define RHI_API
#       pragma warning "Unknown dynamic link import/export semantics."
#   endif
#elif defined(RHI_SHARED_LIBRARY_INCLUDE)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllimport)
#   else
#       define RHI_API
#   endif
#else
#   define RHI_API
#endif

#if defined(_MSC_VER)
#   define RHI_CALL __cdecl
#   pragma warning(disable: 4251)
#else
#   define RHI_CALL
#endif

namespace RHI
{
    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxColorAttachments = 8;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 8;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxCommandLists = 32u;
    static constexpr uint32_t kMaxUniformBufferBindings = 14;
    static constexpr uint32_t kMaxLogMessageSize = 1024;

    static constexpr uint32_t KnownVendorId_AMD = 0x1002;
    static constexpr uint32_t KnownVendorId_Intel = 0x8086;
    static constexpr uint32_t KnownVendorId_Nvidia = 0x10DE;
    static constexpr uint32_t KnownVendorId_Microsoft = 0x1414;
    static constexpr uint32_t KnownVendorId_ARM = 0x13B5;
    static constexpr uint32_t KnownVendorId_ImgTec = 0x1010;
    static constexpr uint32_t KnownVendorId_Qualcomm = 0x5143;

    enum class GraphicsAPI : uint8_t
    {
        Direct3D12,
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

    enum class ShaderFormat : uint8_t
    {
        DXIL,
        SPIRV
    };

    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,

        Count
    };

    enum class LogLevel : uint32_t
    {
        Info = 0,
        Warn,
        Debug,
        Error
    };

    enum class HeapType : uint32_t
    {
        Default,
        Upload,
        Readback,
    };

    /// Defines texture format.
    enum class TextureFormat : uint32_t
    {
        Undefined = 0,
        // 8-bit formats
        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,
        // 16-bit formats
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,
        R16Float,
        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,
        // Packed 16-Bit Pixel Formats
        BGRA4Unorm,
        B5G6R5Unorm,
        B5G5R5A1Unorm,
        // 32-bit formats
        R32Uint,
        R32Sint,
        R32Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RG16Float,
        RGBA8Unorm,
        RGBA8UnormSrgb,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-Bit formats
        RGB10A2Unorm,
        RG11B10Float,
        RGB9E5Float,
        // 64-Bit formats
        RG32Uint,
        RG32Sint,
        RG32Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RGBA16Uint,
        RGBA16Sint,
        RGBA16Float,
        // 128-Bit formats
        RGBA32Uint,
        RGBA32Sint,
        RGBA32Float,
        // Depth-stencil formats
        Depth16Unorm,
        Depth32Float,
        Depth24UnormStencil8,
        Depth32FloatStencil8,
        // Compressed BC formats
        BC1RGBAUnorm,
        BC1RGBAUnormSrgb,
        BC2RGBAUnorm,
        BC2RGBAUnormSrgb,
        BC3RGBAUnorm,
        BC3RGBAUnormSrgb,
        BC4RUnorm,
        BC4RSnorm,
        BC5RGUnorm,
        BC5RGSnorm,
        BC6HRGBFloat,
        BC6HRGBUFloat,
        BC7RGBAUnorm,
        BC7RGBAUnormSrgb,
        // EAC/ETC compressed formats
        ETC2RGB8Unorm,
        ETC2RGB8UnormSrgb,
        ETC2RGB8A1Unorm,
        ETC2RGB8A1UnormSrgb,
        ETC2RGBA8Unorm,
        ETC2RGBA8UnormSrgb,
        EACR11Unorm,
        EACR11Snorm,
        EACRG11Unorm,
        EACRG11Snorm,
        // ASTC compressed formats
        ASTC4x4Unorm,
        ASTC4x4UnormSrgb,
        ASTC5x4Unorm,
        ASTC5x4UnormSrgb,
        ASTC5x5Unorm,
        ASTC5x5UnormSrgb,
        ASTC6x5Unorm,
        ASTC6x5UnormSrgb,
        ASTC6x6Unorm,
        ASTC6x6UnormSrgb,
        ASTC8x5Unorm,
        ASTC8x5UnormSrgb,
        ASTC8x6Unorm,
        ASTC8x6UnormSrgb,
        ASTC8x8Unorm,
        ASTC8x8UnormSrgb,
        ASTC10x5Unorm,
        ASTC10x5UnormSrgb,
        ASTC10x6Unorm,
        ASTC10x6UnormSrgb,
        ASTC10x8Unorm,
        ASTC10x8UnormSrgb,
        ASTC10x10Unorm,
        ASTC10x10UnormSrgb,
        ASTC12x10Unorm,
        ASTC12x10UnormSrgb,
        ASTC12x12Unorm,
        ASTC12x12UnormSrgb,

        Count
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

    enum class PresentMode : uint32_t
    {
        Immediate = 0,
        Mailbox,
        Fifo,
    };


    /* Logging */
    typedef void (RHI_CALL* LogFunction)(LogLevel level, const char* message);

    RHI_API void SetLogFunction(LogFunction function);
    RHI_API void LogInfo(const char* format, ...);
    RHI_API void LogDebug(const char* format, ...);
    RHI_API void LogWarn(const char* format, ...);
    RHI_API void LogError(const char* format, ...);

    /* Structs */
    enum class BufferUsage : uint32_t
    {
        None = 0,
        CopySrc = 1 << 0,
        CopyDst = 1 << 1,
        Vertex = 1 << 2,
        Index = 1 << 3,
        Uniform = 1 << 4,
        ShaderRead = 1 << 5,
        ShaderWrite = 1 << 6,
        Indirect = 1 << 7,
        RayTracingAccelerationStructure = 1 << 8,
        RayTracingShaderTable = 1 << 9,
    };
    RHI_ENUM_CLASS_FLAG_OPERATORS(BufferUsage);

    struct BufferDesc
    {
        uint64_t size = 0;
        BufferUsage usage = BufferUsage::None;
        HeapType heapType = HeapType::Default;
        uintptr_t handle = 0;
        const char* label = nullptr;
    };

    enum class TextureDimension : uint32_t
    {
        Texture1D,
        Texture2D,
        Texture3D,
    };

    enum class TextureUsage : uint32_t
    {
        None = 0,
        CopySrc = 1 << 0,
        CopyDst = 1 << 1,
        Sampled = 1 << 2,
        Storage = 1 << 3,
        RenderTarget = 1 << 4,
        ShadingRate = 1 << 5,
    };
    RHI_ENUM_CLASS_FLAG_OPERATORS(TextureUsage);

    struct TextureDesc
    {
        const char* label = nullptr;
        TextureUsage usage = TextureUsage::Sampled;
        TextureDimension dimension = TextureDimension::Texture2D;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        TextureFormat format = TextureFormat::BGRA8Unorm;
        uint32_t mipLevelCount = 1;
        uint32_t sampleCount = 1;

        static inline TextureDesc Tex1D(
            TextureFormat format,
            uint32_t width,
            uint32_t arrayLayers = 1,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture1D;
            desc.width = width;
            desc.height = 1;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex2D(
            TextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t arrayLayers = 1,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled,
            uint32_t sampleCount = 1) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = sampleCount;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc Tex3D(
            TextureFormat format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipLevelCount = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture3D;
            desc.width = width;
            desc.height = height;
            desc.depthOrArrayLayers = depth;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }

        static inline TextureDesc TexCube(
            TextureFormat format,
            uint32_t size,
            uint32_t mipLevelCount = 1,
            uint32_t arrayLayers = 1,
            TextureUsage usage = TextureUsage::Sampled) noexcept
        {
            TextureDesc desc;
            desc.dimension = TextureDimension::Texture2D;
            desc.width = size;
            desc.height = size;
            desc.depthOrArrayLayers = 6 * arrayLayers;
            desc.mipLevelCount = mipLevelCount;
            desc.format = format;
            desc.sampleCount = 1;
            desc.usage = usage;
            return desc;
        }
    };

    struct TextureData
    {
        const void* pData = nullptr;
        uint32_t    rowPitch = 0;
        uint32_t    slicePitch = 0;
    };

    struct SwapChainDescriptor
    {
        const char* label = nullptr;
        uint32_t width;
        uint32_t height;
        TextureFormat format = TextureFormat::BGRA8UnormSrgb;
        PresentMode presentMode = PresentMode::Fifo;
    };

    struct DeviceFeatures
    {
        /// Whether tessellation support is available.
        bool tessellation = false;
        /// Whether Ray Tracing support is available.
        bool rayTracing = false;
        /// Whether Mesh Shader support is available.
        bool meshShader = false;
        /// Whether VariableRate shading support is available.
        bool variableRateShading = false;
        /// Whether VariableRate shading Tier2 support is available.
        bool variableRateShadingTier2 = false;
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

    class RHI_API IResource
    {
    protected:
        IResource() = default;
        virtual ~IResource() = default;

    public:
        // Non-copyable and non-movable
        IResource(const IResource&) = delete;
        IResource(const IResource&&) = delete;
        IResource& operator=(const IResource&) = delete;
        IResource& operator=(const IResource&&) = delete;

        virtual uint32_t AddRef()
        {
            return ++refCount;
        }

        virtual uint32_t Release()
        {
            uint32_t result = --refCount;
            if (result == 0) {
                delete this;
            }
            return result;
        }

    private:
        std::atomic_uint32_t refCount = 1;
    };

    //////////////////////////////////////////////////////////////////////////
    // RefCountPtr
    // Mostly a copy of Microsoft::WRL::ComPtr<T>
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class RefCountPtr
    {
    public:
        using InterfaceType = T;

    protected:
        InterfaceType* ptr_;
        template<class U> friend class RefCountPtr;

        void InternalAddRef() const noexcept
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        uint32_t InternalRelease() noexcept
        {
            uint32_t ref = 0;
            T* temp = ptr_;

            if (temp != nullptr)
            {
                ptr_ = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    public:
        RefCountPtr() noexcept : ptr_(nullptr)
        {
        }

        RefCountPtr(std::nullptr_t) noexcept : ptr_(nullptr)
        {
        }

        template<class U>
        RefCountPtr(U* other) noexcept : ptr_(other)
        {
            InternalAddRef();
        }

        RefCountPtr(const RefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        // copy ctor that allows to instanatiate class when U* is convertible to T*
        template<class U>
        RefCountPtr(const RefCountPtr<U>& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)

        {
            InternalAddRef();
        }

        RefCountPtr(RefCountPtr&& other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        // Move ctor that allows instantiation of a class when U* is convertible to T*
        template<class U>
        RefCountPtr(RefCountPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        ~RefCountPtr() noexcept
        {
            InternalRelease();
        }

        RefCountPtr& operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        RefCountPtr& operator=(T* other) noexcept
        {
            if (ptr_ != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(U* other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(const RefCountPtr& other) noexcept  // NOLINT(bugprone-unhandled-self-assignment)
        {
            if (ptr_ != other.ptr_)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(const RefCountPtr<U>& other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(RefCountPtr<U>&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(RefCountPtr&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(RefCountPtr& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        [[nodiscard]] T* Get() const noexcept
        {
            return ptr_;
        }

        operator T* () const
        {
            return ptr_;
        }

        InterfaceType* operator->() const noexcept
        {
            return ptr_;
        }

        T** operator&()   // NOLINT(google-runtime-operator)
        {
            return &ptr_;
        }

        [[nodiscard]] T* const* GetAddressOf() const noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** GetAddressOf() noexcept
        {
            return &ptr_;
        }

        [[nodiscard]] T** ReleaseAndGetAddressOf() noexcept
        {
            InternalRelease();
            return &ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        // Set the pointer while keeping the object's reference count unchanged
        void Attach(InterfaceType* other)
        {
            if (ptr_ != nullptr)
            {
                auto ref = ptr_->Release();
                (void)ref;

                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                assert(ref != 0 || ptr_ != other);
            }

            ptr_ = other;
        }

        // Create a wrapper around a raw object while keeping the object's reference count unchanged
        static RefCountPtr<T> Create(T* other)
        {
            RefCountPtr<T> Ptr;
            Ptr.Attach(other);
            return Ptr;
        }

        uint32_t Reset()
        {
            return InternalRelease();
        }
    };

    class RHI_API IBuffer : public IResource
    {
    public:
        [[nodiscard]] virtual uint64_t GetSize() const = 0;
        [[nodiscard]] virtual BufferUsage GetUsage() const = 0;

        [[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
        [[nodiscard]] virtual uint64_t GetDeviceAddress() const = 0;

        [[nodiscard]] virtual uint8_t* MappedData() const = 0;
    };

    class RHI_API ITexture : public IResource
    {
    public:
        [[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;
    };

    class RHI_API ISwapChain : public IResource
    {
    public:
        virtual bool Resize(uint32_t width, uint32_t height) = 0;
    };

    class RHI_API ICommandList
    {
    protected:
        ICommandList() = default;

    public:
        virtual ~ICommandList() = default;

        // Non-copyable and non-movable
        ICommandList(const ICommandList&) = delete;
        ICommandList(const ICommandList&&) = delete;
        ICommandList& operator=(const ICommandList&) = delete;
        ICommandList& operator=(const ICommandList&&) = delete;

        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const char* name) = 0;

        virtual void BeginRenderPass(const ISwapChain* swapChain, const float clearColor[4]) = 0;
        virtual void EndRenderPass() = 0;
    };

    using BufferHandle = RefCountPtr<IBuffer>;
    using TextureHandle = RefCountPtr<ITexture>;
    using SwapChainHandle = RefCountPtr<ISwapChain>;

    class RHI_API IDevice : public IResource
    {
    public:
        virtual void WaitIdle() = 0;
        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;

        [[nodiscard]] virtual ICommandList* BeginCommandList(CommandQueue queue = CommandQueue::Graphics) = 0;

        /// Create new Texture.
        [[nodiscard]] TextureHandle CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr);

        /// Create new Texture.
        [[nodiscard]] TextureHandle CreateExternalTexture(const void* handle, const TextureDesc& desc);

        /// Create new Buffer.
        [[nodiscard]] BufferHandle CreateBuffer(const BufferDesc& desc, const void* initialData);

        /// Create new SwapChain.
        [[nodiscard]] SwapChainHandle CreateSwapChain(void* windowHandle, const SwapChainDescriptor* desc);

        /// Returns the set of features supported by this device.
        const DeviceFeatures& GetFeatures() const { return features; }

        /// Returns the set of hardware limits for this device.
        const DeviceLimits& GetLimits() const { return limits; }

        ShaderFormat GetShaderFormat() const { return shaderFormat; }

        [[nodiscard]] virtual uint64_t GetFrameCount() const { return frameCount; }
        [[nodiscard]] virtual uint32_t GetFrameIndex() const { return frameIndex; }

    private:
        virtual TextureHandle CreateTextureCore(const TextureDesc& desc, const void* handle, const TextureData* initialData) = 0;
        virtual BufferHandle CreateBufferCore(const BufferDesc& desc, const void* initialData) = 0;
        virtual SwapChainHandle CreateSwapChainCore(void* windowHandle, const SwapChainDescriptor* desc) = 0;

    protected:
        DeviceFeatures features{};
        DeviceLimits limits{};
        ShaderFormat shaderFormat{};
        uint64_t frameCount = 0;
        uint32_t frameIndex = 0;
    };

    using DeviceHandle = RefCountPtr<IDevice>;

    extern RHI_API DeviceHandle GRHIDevice;

    RHI_API DeviceHandle CreateDevice(GraphicsAPI api, ValidationMode validationMode = ValidationMode::Disabled);

    /* Helper methods */
    RHI_API const char* GetVendorName(uint32_t vendorId);

    // Returns the number of mip levels given a texture size
    RHI_API uint32_t CalculateMipLevels(uint32_t width, uint32_t height, uint32_t depth = 1);
}

#undef RHI_ENUM_CLASS_FLAG_OPERATORS

namespace std
{
}
