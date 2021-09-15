// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/SwapChain.h"
#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"
#include "Core/Assert.h"

namespace Alimer
{
	SwapChain::SwapChain(const SwapChainCreateInfo& info)
		: width(info.width)
		, height(info.height)
		, colorFormat(info.colorFormat)
		, presentMode(info.presentMode)
	{
	}

    SwapChainRef SwapChain::Create(void* window, const SwapChainCreateInfo& info)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());
        ALIMER_ASSERT(window != nullptr);

        return gGraphics().CreateSwapChain(window, info);
    }

	void SwapChain::Resize(uint32_t width_, uint32_t height_)
	{
		width = width_;
		height = height_;

		ResizeBackBuffer(width, height);
	}
}
