// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "D3D12Utils.h"

namespace Alimer
{
	class D3D12Buffer final : public Buffer, public D3D12GpuResource
	{
	public:
		D3D12Buffer(D3D12Graphics& device, const BufferCreateInfo& info, const void* initialData);
		~D3D12Buffer() override;
		void Destroy() override;

		uint8_t* Map() override;
		void Unmap() override;
		void Update(const void* data, size_t size, size_t offset = 0);

    private:
		void ApiSetName() override;

	private:
		D3D12Graphics& device;

		uint8_t* mappedData{ nullptr };

		/// Whether the buffer has been mapped with vmaMapMemory
		bool mapped{ false };
	};

	constexpr D3D12Buffer* ToD3D12(Buffer* resource)
	{
		return static_cast<D3D12Buffer*>(resource);
	}

	constexpr const D3D12Buffer* ToD3D12(const Buffer* resource)
	{
		return static_cast<const D3D12Buffer*>(resource);
	}
}

