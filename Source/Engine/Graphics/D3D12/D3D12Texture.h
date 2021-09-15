// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Texture.h"
#include "D3D12Utils.h"

namespace Alimer
{
	class D3D12SwapChain;

	class D3D12Texture final : public Texture, public D3D12GpuResource
	{
	public:
		D3D12Texture(D3D12Graphics& device, const TextureCreateInfo& info, const void* initialData, ID3D12Resource* existingHandle = nullptr, PixelFormat viewFormat = PixelFormat::Undefined);
		~D3D12Texture() override;
		void Destroy() override;

		// Swapchain texture logic.
		bool IsSwapChainTexture() const { return swapChain != nullptr; }
		D3D12SwapChain* GetSwapChain() const { return swapChain; }
		void SetSwapChain(D3D12SwapChain* swapChain_) { swapChain = swapChain_; }

        D3D12Graphics& GetDevice() { return device; }
		DXGI_FORMAT GetDXGIFormat() const noexcept { return dxgiFormat; }

        TextureView* CreateView(const TextureViewCreateInfo& createInfo) override;

	private:
        void ApiSetName() override;

		D3D12Graphics& device;
		D3D12SwapChain* swapChain = nullptr;
		DXGI_FORMAT dxgiFormat;
	};

    class D3D12TextureView final : public TextureView
    {
    public:
        D3D12TextureView(_In_ D3D12Texture* texture, const TextureViewCreateInfo& info);
        ~D3D12TextureView() override;

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV() const { return rtv; }
        const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return srv; }
        const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV(bool readOnlyDepth, bool readOnlyStencil) const;

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE CreateRTV(ID3D12Resource* resource) const;
        D3D12_CPU_DESCRIPTOR_HANDLE CreateDSV(ID3D12Resource* resource, bool readOnlyDepth, bool readOnlyStencil) const;

        D3D12Graphics& device;
        mutable D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
        D3D12_CPU_DESCRIPTOR_HANDLE readWriteDSV{};
        mutable D3D12_CPU_DESCRIPTOR_HANDLE readWriteDepthReadOnlyView{};
        mutable D3D12_CPU_DESCRIPTOR_HANDLE readOnlyDepthReadWriteStencilView{};
        mutable D3D12_CPU_DESCRIPTOR_HANDLE readOnlyDSV{};
        D3D12_CPU_DESCRIPTOR_HANDLE srv{};
    };
}

