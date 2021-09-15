// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/SwapChain.h"
#include "D3D12Utils.h"
#include <vector>

namespace Alimer
{
	class D3D12SwapChain final : public SwapChain
	{
	public:
		D3D12SwapChain(D3D12Graphics& device, void* windowHandle, const SwapChainCreateInfo& info);
		~D3D12SwapChain() override;
		void Destroy() override;

		HRESULT Present();

		IDXGISwapChain3* GetHandle() const { return handle; }

	private:
		void ResizeBackBuffer(uint32_t width, uint32_t height) override;
		void UpdateColorSpace();
        TextureView* GetCurrentTextureView() const override;

		D3D12Graphics& device;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		HWND window = nullptr;
#else
		IUnknown* window = nullptr;
#endif

		UINT syncInterval = 1;
		UINT flags = 0;
		IDXGISwapChain3* handle = nullptr;

		// HDR Support
		DXGI_COLOR_SPACE_TYPE colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };
		DXGI_MODE_ROTATION swapChainRotation{ DXGI_MODE_ROTATION_IDENTITY };

		std::vector<RefCountPtr<D3D12Texture>> colorTextures;
	};
}

