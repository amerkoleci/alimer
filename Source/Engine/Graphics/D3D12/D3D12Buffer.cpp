// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Buffer.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Graphics.h"

namespace Alimer
{
	D3D12Buffer::D3D12Buffer(D3D12Graphics& device_, const BufferCreateInfo& info, const void* initialData)
		: Buffer(info)
		, device(device_)
	{
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = size;
        if ((usage & BufferUsage::Uniform) != 0)
        {
            // Align the buffer size to multiples of the dynamic uniform buffer minimum size
            resourceDesc.Width = AlignUp(resourceDesc.Width, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (!CheckBitsAny(usage, BufferUsage::ShaderRead))
		{
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}
		if (CheckBitsAny(usage, BufferUsage::ShaderWrite))
		{
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		switch (memoryUsage)
		{
		case MemoryUsage::CpuOnly:
		case MemoryUsage::CpuToGpu:
			allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
			fixedResourceState = true;
			break;

		case MemoryUsage::GpuToCpu:
			allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
			fixedResourceState = true;
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			break;

		default:
			allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
			break;
		}

		HRESULT result = device.GetAllocator()->CreateResource(&allocDesc,
			&resourceDesc,
			state,
			nullptr,
			&allocation,
			IID_PPV_ARGS(&handle)
		);

		if (FAILED(result))
		{
			LOGE("Direct3D12: Could not create buffer: {}", std::to_string(result));
			return;
		}

        if (info.label != nullptr)
        {
            SetName(info.label);
        }

        device.GetHandle()->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &allocatedSize);

		if (memoryUsage != MemoryUsage::GpuOnly)
		{
			ThrowIfFailed(handle->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
		}

		if (initialData != nullptr)
		{
			if (memoryUsage == MemoryUsage::GpuOnly)
			{
				D3D12UploadContext uploadContext = device.ResourceUploadBegin(resourceDesc.Width);
				memcpy(uploadContext.CPUAddress, initialData, size);
				
				uploadContext.commandList->CopyBufferRegion(handle, 0,
                    uploadContext.resource, uploadContext.resourceOffset, size);

				device.ResourceUploadEnd(uploadContext);
			}
			else
			{
				memcpy(mappedData, initialData, size);
			}
		}

		gpuVirtualAddress = handle->GetGPUVirtualAddress();
        OnCreated();
	}

	D3D12Buffer::~D3D12Buffer()
	{
		Destroy();
	}

	void D3D12Buffer::Destroy()
	{
		if (handle != nullptr
			&& allocation != nullptr)
		{
			Unmap();
			device.DeferDestroy(handle, allocation);
		}

        D3D12GpuResource::Destroy();
        OnDestroyed();
	}

	void D3D12Buffer::ApiSetName()
	{
		auto wideName = ToUtf16(name);
        handle->SetName(wideName.c_str());
	}

	uint8_t* D3D12Buffer::Map()
	{
		if (!mapped && !mappedData)
		{
			D3D12_RANGE readRange = { };
			ThrowIfFailed(handle->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
			mapped = true;
		}

		return mappedData;
	}

	void D3D12Buffer::Unmap()
	{
		if (mapped)
		{
            handle->Unmap(0, nullptr);
			mappedData = nullptr;
			mapped = false;
		}
	}

	void D3D12Buffer::Update(const void* data, size_t size, size_t offset)
	{
		if (memoryUsage == MemoryUsage::GpuOnly)
		{
		}
		else
		{
			memcpy(mappedData, data, size);
		}
	}
}
