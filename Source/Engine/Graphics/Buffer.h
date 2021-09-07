// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"

namespace Alimer
{
    enum class BufferUsage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Constant = 1 << 2,
        ShaderRead = 1 << 3,
        ShaderWrite = 1 << 4,
        Indirect = 1 << 5,
        RayTracingAccelerationStructure = 1 << 6,
        RayTracingShaderTable = 1 << 7,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(BufferUsage);

    struct BufferDesc
    {
        const char* label = nullptr;
        uint64_t size = 0;
        BufferUsage usage = BufferUsage::None;
        HeapType heapType = HeapType::Default;
        //uint32_t stride = 0;
        //PixelFormat format = PixelFormat::Undefined;
    };

    class ALIMER_API Buffer : public GraphicsResource
    {
    public:
        [[nodiscard]] static BufferRef Create(const BufferDesc& desc, const void* initialData = nullptr);
        [[nodiscard]] static BufferRef Create(const void* data, uint64_t size, BufferUsage usage, const char* label = nullptr);

        [[nodiscard]] uint64_t GetSize() const noexcept { return size; }
        [[nodiscard]] BufferUsage GetUsage() const noexcept { return usage; }
        [[nodiscard]] virtual uint64_t GetDeviceAddress() const = 0;
        
        [[nodiscard]] virtual uint8_t* MappedData() const = 0;

    protected:
        /// Constructor.
        Buffer(const BufferDesc& desc);

        uint64_t size;
        BufferUsage usage;
    };
}

