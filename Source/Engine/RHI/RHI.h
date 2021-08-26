// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

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

namespace alimer::RHI
{
    /* Constants */
    static constexpr uint32_t kMaxFramesInFlight = 2;
    static constexpr uint32_t kMaxSimultaneousRenderTargets = 8;
    static constexpr uint32_t kMaxFrameCommandBuffers = 32;
    static constexpr uint32_t kMaxViewportsAndScissors = 8;
    static constexpr uint32_t kMaxVertexBufferBindings = 4;
    static constexpr uint32_t kMaxVertexAttributes = 16;
    static constexpr uint32_t kMaxVertexAttributeOffset = 2047u;
    static constexpr uint32_t kMaxVertexBufferStride = 2048u;
    static constexpr uint32_t kMaxUniformBufferBindings = 14;
    static constexpr uint32_t kMaxDescriptorBindings = 32;
    static constexpr uint32_t kMaxUniformBufferSize = 16 * 1024;
    static constexpr uint32_t kInvalidBindlessIndex = static_cast<uint32_t>(-1);

    static constexpr uint32_t KnownVendorId_AMD = 0x1002;
    static constexpr uint32_t KnownVendorId_Intel = 0x8086;
    static constexpr uint32_t KnownVendorId_Nvidia = 0x10DE;
    static constexpr uint32_t KnownVendorId_Microsoft = 0x1414;
    static constexpr uint32_t KnownVendorId_ARM = 0x13B5;
    static constexpr uint32_t KnownVendorId_ImgTec = 0x1010;
    static constexpr uint32_t KnownVendorId_Qualcomm = 0x5143;

    /* Enums */
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

    /* Forward declarations */
    class IBuffer;
    class ITexture;
    class ISampler;
    class IShader;
    class IDevice;

    using DeviceHandle = RefCountPtr<IDevice>;

    class IDevice : public RefCounted
    {
    public:
        static DeviceHandle Create(void* windowHandle);

        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;
    };

    /* Helper methods */
    RHI_API const char* ToString(CompareFunction func);

    template <class T>
    void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

#undef RHI_ENUM_CLASS_FLAG_OPERATORS

#if TODO
namespace std
{
    template<> struct hash<Alimer::RHI::TextureSubresourceSet>
    {
        std::size_t operator()(const Alimer::RHI::TextureSubresourceSet& set) const noexcept
        {
            size_t hash = 0;
            Alimer::RHI::hash_combine(hash, set.baseMipLevel);
            Alimer::RHI::HashCombine(hash, set.numMipLevels);
            Alimer::RHI::HashCombine(hash, set.baseArraySlice);
            Alimer::RHI::HashCombine(hash, set.numArraySlices);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::StencilFaceState>
    {
        std::size_t operator()(const Alimer::RHI::StencilFaceState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, (uint32_t)state.failOp);
            Alimer::HashCombine(hash, (uint32_t)state.passOp);
            Alimer::HashCombine(hash, (uint32_t)state.depthFailOp);
            Alimer::HashCombine(hash, (uint32_t)state.compare);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::DepthStencilState>
    {
        std::size_t operator()(const Alimer::RHI::DepthStencilState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.depthWriteEnabled);
            Alimer::HashCombine(hash, (uint32_t)state.depthCompare);
            Alimer::HashCombine(hash, state.frontFace);
            Alimer::HashCombine(hash, state.backFace);
            Alimer::HashCombine(hash, state.stencilReadMask);
            Alimer::HashCombine(hash, state.stencilWriteMask);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::RenderTargetBlendState>
    {
        std::size_t operator()(const Alimer::RHI::RenderTargetBlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.blendEnable);
            Alimer::HashCombine(hash, (uint32_t)state.srcBlend);
            Alimer::HashCombine(hash, (uint32_t)state.destBlend);
            Alimer::HashCombine(hash, (uint32_t)state.blendOp);
            Alimer::HashCombine(hash, (uint32_t)state.srcBlendAlpha);
            Alimer::HashCombine(hash, (uint32_t)state.destBlendAlpha);
            Alimer::HashCombine(hash, (uint32_t)state.blendOpAlpha);
            Alimer::HashCombine(hash, (uint8_t)state.writeMask);
            return hash;
        }
    };

    template<> struct hash<Alimer::RHI::BlendState>
    {
        std::size_t operator()(const Alimer::RHI::BlendState& state) const noexcept
        {
            size_t hash = 0;
            Alimer::HashCombine(hash, state.alphaToCoverageEnable);
            Alimer::HashCombine(hash, state.independentBlendEnable);
            for (const auto& target : state.renderTargets)
            {
                Alimer::HashCombine(hash, target);
            }
            return hash;
        }
    };
}
#endif // TODO

