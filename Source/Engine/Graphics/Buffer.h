// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GPUResource.h"

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

    struct BufferCreateInfo
    {
        const char* label = nullptr;
        uint64_t size = 0;
        BufferUsage usage = BufferUsage::None;
        CpuAccessMode cpuAccess = CpuAccessMode::None;
        PixelFormat format = PixelFormat::Undefined;
        uintptr_t handle = 0;
    };

	class ALIMER_API Buffer : public GPUResource, public RefCounted
	{
	public:
        [[nodiscard]] static BufferRef Create(const BufferCreateInfo* createInfo, const void* initialData = nullptr);
		[[nodiscard]] static BufferRef Create(uint64_t size, BufferUsage usage, const void* data, const char* label = nullptr);
        [[nodiscard]] static BufferRef CreateUpload(uint64_t size, const char* label = nullptr);
        [[nodiscard]] static BufferRef CreateReadback(uint64_t size, const char* label = nullptr);

        [[nodiscard]] uint64_t GetSize() const noexcept { return size; }
        [[nodiscard]] BufferUsage GetUsage() const noexcept { return usage; }

        [[nodiscard]] virtual uint64_t GetDeviceAddress() const = 0;
        [[nodiscard]] virtual uint8_t* MappedData() const = 0;

	protected:
		/// Constructor.
		Buffer(const BufferCreateInfo* info);

        uint64_t size;
		BufferUsage usage;
        uint64_t deviceAddress{ 0 };
        uint8_t* mappedData{ nullptr };
	};
}
